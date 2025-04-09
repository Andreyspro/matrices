#include <iostream>
#include "mtxsolver_async.h"
#include "and_net3.h"

#define noEXTRAOUT
#define EXTRAOUT2

mtxsolver_async_loader::mtxsolver_async_loader(std::shared_ptr<and_net::net_three> net_ptr)
	: m_net_ptr(net_ptr), m_stage(LOAD_TYPE), m_load_index({0,0})
{
}

mtxsolver_async_loader::self_ptr_t mtxsolver_async_loader::get_new(std::shared_ptr<and_net::net_three> &net)
{
	return self_ptr_t(new mtxsolver_async_loader(net));
}


void mtxsolver_async_loader::start(callback_t callback)
{
	m_callback = callback;
	do_read_mtx();
}

void mtxsolver_async_loader::do_read_mtx()
{
	on_read_mtx(and_net::op_fill_status_t{});
}

void mtxsolver_async_loader::on_read_mtx(and_net::op_fill_status_t fill_status)
{
	#ifdef EXTRAOUT
	std::cout << "on_read_mtx. Start. Fill status is - " <<  fill_status.fill_result 
		<<" | " << fill_status.reason << std::endl;
	#endif
	m_invokes++;
	if (fill_status.fill_result != and_net::FILL_OK)
	{
		#ifdef EXTRAOUT
		std::cout << "on_read_mtx. ERROR - FILL not ok" << std::endl;
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
			#ifdef EXTRAOUT
			std::cout << "on_read_mtx. type is " << read_str << std::endl;
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
			#ifdef EXTRAOUT
			std::cout << "on_read_mtx. version is - " << read_str << std::endl;
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
			#ifdef EXTRAOUT
			std::cout << "on_read_mtx. subversion is " << read_str << std::endl;
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
			#ifdef EXTRAOUT
			std::cout << "on_read_mtx. size is " << read_str << std::endl;
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
			#ifdef EXTRAOUT
			std::cout << "on_read_mtx. element [" << m_load_index.i <<
			
				"][" << m_load_index.j << "] = " << read_str << std::endl;
			#endif
			// stod - is CPU very expensive operation !
			Mtx[m_load_index.i][m_load_index.j] = std::stod(read_str);

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
		#ifdef EXTRAOUT
		std::cout << "on_read_mtx. LOAD done" << std::endl;
		#endif
		std::cout << "invokes = . " << m_invokes << std::endl;
		m_callback(true, size, shared_from_this());
		return;
	}
	else
	{
		#ifdef EXTRAOUT
		std::cout << "on_read_mtx. ERROR READ not OK" << std::endl;
		#endif
		m_callback(false, 0, nullptr);
		return;
	}
}
