#include <iostream>
#include "mtxsolver_async.h"
#include "and_net3.h"
#include "uuid.h"
#include "crack_atod.h"

#define NOMTX_ASYNC_SOLVER_EXTRAOUT2

mtxsolver_async_loader::mtxsolver_async_loader(std::shared_ptr<and_net::net_three> net_ptr)
	: m_net_ptr(net_ptr), m_stage(LOAD_TYPE), m_load_index({0,0})
{
	#ifdef MTX_ASYNC_SOLVER_EXTRAOUT
	m_id = new_uuid();
	std::string prefix = "mtxsolver_async_loader (" + m_id + ") mtxsolver_async_loader. ";
	std::cout << prefix << "CTOR"  << std::endl << std::endl;
	#endif
}

mtxsolver_async_loader::self_ptr_t mtxsolver_async_loader::get_new(std::shared_ptr<and_net::net_three> net)
{
	return self_ptr_t(new mtxsolver_async_loader(net));
}


void mtxsolver_async_loader::start(callback_t callback)
{
	#ifdef MTX_ASYNC_SOLVER_EXTRAOUT
	std::string prefix = "mtxsolver_async_loader (" + m_id + ") start. ";
	std::cout << prefix << "Start." << std::endl;
	#endif

	m_callback = callback;
	do_read_mtx();

	#ifdef MTX_ASYNC_SOLVER_EXTRAOUT
	std::cout << prefix << "End" << std::endl;
	#endif
}

void mtxsolver_async_loader::do_read_mtx()
{
	#ifdef MTX_ASYNC_SOLVER_EXTRAOUT
	std::string prefix = "mtxsolver_async_loader (" + m_id + ") do_read_mtx. ";
	std::cout << prefix << "Start." << std::endl;
	#endif

	on_read_mtx(and_net::op_fill_status_t{});

	#ifdef MTX_ASYNC_SOLVER_EXTRAOUT
	std::cout << prefix << "End" << std::endl;
	#endif
}

void mtxsolver_async_loader::on_read_mtx(and_net::op_fill_status_t fill_status)
{
	#ifdef MTX_ASYNC_SOLVER_EXTRAOUT
	std::string prefix = "mtxsolver_async_loader (" + m_id + ") on_read_mtx. ";
	std::cout << prefix << "Start." << std::endl;
	#endif

	#ifdef MTX_ASYNC_SOLVER_EXTRAOUT2
	std::cout << "on_read_mtx. Start. Fill status is - " <<  fill_status.fill_result 
		<<" | " << fill_status.reason << std::endl;
	#endif

	m_invokes++;
	if (fill_status.fill_result != and_net::FILL_OK)
	{
		#ifdef MTX_ASYNC_SOLVER_EXTRAOUT
		std::cout << prefix << "Fill not ok" << std::endl;
		#endif

		m_callback(false, 0, nullptr);
		return;
	}

	and_net::op_read_status_t read_status;
	std::string read_str;
	if (m_stage == LOAD_TYPE)
	{
		read_status = m_net_ptr->try_read_str_em(read_str, "\n");
		if (read_status.read_result == and_net::READ_OK)
		{
			#ifdef MTX_ASYNC_SOLVER_EXTRAOUT
			std::cout << prefix << "type is " << read_str << std::endl;
			#endif

			std::strcpy(data_header.type, read_str.substr(0,16).c_str());
			m_stage = LOAD_VERSION;
		}
	}

	if (m_stage == LOAD_VERSION)
	{
		read_status = m_net_ptr->try_read_str_em(read_str, "\n");
		if (read_status.read_result == and_net::READ_OK)
		{
			#ifdef MTX_ASYNC_SOLVER_EXTRAOUT
			std::cout << prefix << "version is - " << read_str << std::endl;
			#endif

			data_header.version = std::stoi(read_str);
			m_stage = LOAD_SUBVERSION;
		}
	}

	if (m_stage == LOAD_SUBVERSION)
	{
		read_status = m_net_ptr->try_read_str_em(read_str, "\n");
		if (read_status.read_result == and_net::READ_OK)
		{
			#ifdef MTX_ASYNC_SOLVER_EXTRAOUT
			std::cout << prefix << "subversion is " << read_str << std::endl;
			#endif

			data_header.version = std::stoi(read_str);
			m_stage = LOAD_SIZE;
		}
	}

	if (m_stage == LOAD_SIZE)
	{
		read_status = m_net_ptr->try_read_str_em(read_str, "\n");
		if (read_status.read_result == and_net::READ_OK)
		{
			#ifdef MTX_ASYNC_SOLVER_EXTRAOUT
			std::cout << prefix << "size is " << read_str << std::endl;
			#endif

			size = std::stoi(read_str);
			Mtx.reserve(size);
			for (size_t j = 0; j < size; j++)
			{
				Mtx.emplace_back(size + 1);
			}
			m_stage = LOAD_MTX_ELEMENTS;
		}
	}
	if (m_stage == LOAD_MTX_ELEMENTS)
	{
		while 
		(
			m_stage == LOAD_MTX_ELEMENTS &&
			(read_status = m_net_ptr->try_read_str_em(read_str, "\n")).read_result == and_net::READ_OK
		)
		{
			#ifdef MTX_ASYNC_SOLVER_EXTRAOUT2
			std::cout << "on_read_mtx. element [" << m_load_index.i <<
			
				"][" << m_load_index.j << "] = " << read_str << std::endl;
			#endif

			// stod - is CPU very expensive operation !
			// Mtx[m_load_index.i][m_load_index.j] = std::stod(read_str);
			// Mtx[m_load_index.i][m_load_index.j] = 123456.98765;
			// Mtx[m_load_index.i][m_load_index.j] = std::strtod(read_str.c_str(), NULL);
			Mtx[m_load_index.i][m_load_index.j] = crack_atod::atod(&read_str.front(), &read_str.back());
			
			if (m_load_index.j < size)
				m_load_index.j++;
			else
			{
				m_load_index.i++;
				m_load_index.j = 0;
			}
			if (m_load_index.i >= size)
			{
				m_stage = LOAD_DONE;
			}
		}
	}

	if 
	(
		m_stage < LOAD_DONE && read_status.read_result == and_net::READ_FAILED &&
		read_status.reason == and_net::NEED_MORE_DATA
	)
	{
		m_net_ptr->async_fill_buff
		(
			std::bind
			(
				&mtxsolver_async_loader::on_read_mtx,
				shared_from_this(),
				std::placeholders::_1
			)
		);
		return;
	}
	else if (m_stage == LOAD_DONE)
	{
		#ifdef MTX_ASYNC_SOLVER_EXTRAOUT
		std::cout << prefix << "LOAD done" << std::endl;
		#endif

		std::cout << "invokes = " << m_invokes << std::endl;
		m_callback(true, size, shared_from_this());
		return;
	}
	else
	{
		#ifdef MTX_ASYNC_SOLVER_EXTRAOUT
		std::cout << prefix << "ERROR READ not OK" << std::endl;
		#endif
		
		m_callback(false, 0, shared_from_this());
		return;
	}
}

mtxsolver_async_loader::~mtxsolver_async_loader()
{
	#ifdef MTX_ASYNC_SOLVER_EXTRAOUT
	std::string prefix = "mtxsolver_async_loader (" + m_id + ") ~mtxsolver_async_loader. ";
	std::cout << prefix << "DTOR"  << std::endl << std::endl;
	#endif
}
