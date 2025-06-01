#include "mtx_net_handler.h"
#include <functional>
#include <string>

#include "and_net3.h"
#include "uuid.h"

using namespace boost::asio;

namespace mtx
{

mtx_net_handler::mtx_net_handler(io_service &service)
	: m_sock(service), m_stage(STAGE_READ_HELLO)
{
	#ifdef MTX_ASYNC_SOLVER_EXTRAOUT
	m_id = new_uuid();
	std::string prefix = "mtx_net_handler (" + m_id + ") mtx_net_handler. ";
	std::cout << prefix << "CTOR" << std::endl;
	#endif
}

mtx_net_handler::ptr_t mtx_net_handler::new_instance(io_service &service)
{
	return ptr_t(new mtx_net_handler(service));
}

ip::tcp::socket &mtx_net_handler::get_socket()
{
	return m_sock;
}

void mtx_net_handler::start_handle_connection(mtx_load_complete_cb_t cb)
{
	#ifdef MTX_ASYNC_SOLVER_EXTRAOUT
	std::string prefix = "mtx_net_handler (" + m_id + ") start_handle_connection. ";
	std::cout << prefix << "Start." << std::endl;
	#endif
	// std::cout << m_sock.remote_endpoint().address().to_string() << std::endl;
	#ifdef MTX_NET_HANDLER_LOAD_BENCMARK
	bench_timer.restart();
	#endif
	m_load_complete_cb = cb;
	m_anet = and_net::net_three::get_new(&m_sock, 2048);
	// m_mtx_loader = mtxsolver_async_loader::get_new(m_anet);
	and_net::op_fill_status_t fill{and_net::FILL_OK, and_net::FILL_NO_ERROR};
	read_hello(fill);

	#ifdef MTX_ASYNC_SOLVER_EXTRAOUT
	std::cout << prefix << "End." << std::endl;
	#endif
}

void mtx_net_handler::read_hello(and_net::op_fill_status_t fill_status)
{
	#ifdef MTX_ASYNC_SOLVER_EXTRAOUT
	std::string prefix = "mtx_net_handler (" + m_id + ") read_hello. ";
	std::cout << prefix << "Start." << std::endl;
	#endif

	if (fill_status.fill_result != and_net::FILL_OK)
	{

		#ifdef MTX_ASYNC_SOLVER_EXTRAOUT
		std::cout << prefix << "Error. fill not ok!" << std::endl;
		#endif
		std::cout << "Error. fill not ok!" << std::endl;

		return;
	}
	std::string read_str;
	and_net::op_read_status_t read_status;

	read_status = m_anet->try_read_str_em(read_str, "\n");
	if (read_status.read_result == and_net::READ_OK)
	{
		mtx::version_t mtx_version = mtx::parse_net_hello(read_str);
		if (mtx_version > 0 && mtx_version <= mtx::MTX_NET_SUPPORTED_VER)
		{

			#ifdef MTX_ASYNC_SOLVER_EXTRAOUT
			std::cout << prefix << "HELLO header is OK." << std::endl;
			#endif

			m_stage = STAGE_READ_COMMAND;
			read_command(and_net::op_fill_status_t{});
		}
	}
	else if (read_status.reason == and_net::NEED_MORE_DATA)
	{
		m_anet->async_fill_buff
		(
			std::bind
			(
				&mtx_net_handler::read_hello,
				shared_from_this(),
				std::placeholders::_1
			)
		);
		return;
	}
	else
	{
		#ifdef MTX_ASYNC_SOLVER_EXTRAOUT
		std::cout << prefix << "ERROR read not ok" << std::endl;
		#endif
		return;
	}
}

void mtx_net_handler::read_command(and_net::op_fill_status_t fill_status)
{
	#ifdef MTX_ASYNC_SOLVER_EXTRAOUT
	std::string prefix = "mtx_net_handler (" + m_id + ") read_command. ";
	std::cout << prefix << "Start." << std::endl;
	#endif
	
	if (fill_status.fill_result != and_net::FILL_OK)
	{
		#ifdef MTX_ASYNC_SOLVER_EXTRAOUT
		std::cout << prefix << "Error.fill not ok!." << std::endl;
		#endif
		std::cout << "Error. read_command, fill not ok!" << std::endl;
		return;
	}
	std::string read_str;
	and_net::op_read_status_t read_status;

	read_status = m_anet->try_read_str_em(read_str, "\n");
	if (read_status.read_result == and_net::READ_OK)
	{
		if (read_str == MTX_NET_CMD_RECEIVE_MTX_AND_CALC)
		{
			#ifdef MTX_ASYNC_SOLVER_EXTRAOUT
			std::cout << prefix << "command is RECEIVE_MTX_AND_CALC." << std::endl;
			#endif
			m_stage = STAGE_LOAD_MTX;
			mtxsolver_async_loader::self_ptr_t mtx_loader = mtxsolver_async_loader::get_new(m_anet);
			mtx_loader->set_name(m_sock.remote_endpoint().address().to_string());
			mtx_loader->start
			(
				std::bind
				(
					&mtx_net_handler::on_load_mtx,
					shared_from_this(),
					std::placeholders::_1,
					std::placeholders::_2,
					std::placeholders::_3
				)
			);
		}
	}
	else if (read_status.reason == and_net::NEED_MORE_DATA)
	{
		m_anet->async_fill_buff
		(
			std::bind
			(
				&mtx_net_handler::read_command,
				shared_from_this(),
				std::placeholders::_1
			)
		);
		return;
	}
	else
	{
		#ifdef MTX_ASYNC_SOLVER_EXTRAOUT
		std::cout << prefix << "ERROR read not ok" << std::endl;
		#endif
		return;
	}
}

void mtx_net_handler::on_load_mtx(bool load_ok, size_t mtx_size, mtxsolver_async_loader::self_ptr_t mtx_loader_ptr)
{
	#ifdef MTX_NET_HANDLER_LOAD_BENCMARK
	bench_timer.stop();
	std::cout << "Load time is " << bench_timer.get_duration() << " mSec." << std::endl;
	#endif
	#ifdef MTX_ASYNC_SOLVER_EXTRAOUT
	std::string prefix = "mtx_net_handler (" + m_id + ") on_load_mtx. ";
	std::cout << prefix << "Load ok, size is - " << mtx_size << std::endl << std::endl;
	std::cout << "Load ok, size is - " << mtx_size << std::endl;
	#endif
	mtx_loader_ptr->set_name(mtx_loader_ptr->get_name() + "(" + std::to_string(mtx_loader_ptr->getSize()) + ")" +
		new_uuid());
	m_load_complete_cb(*mtx_loader_ptr);
}

mtx_net_handler::~mtx_net_handler()
{

	#ifdef MTX_ASYNC_SOLVER_EXTRAOUT
	std::string prefix = "mtx_net_handler (" + m_id + ") ~mtx_net_handler. ";
	std::cout << prefix << "DTOR"  << std::endl << std::endl;
	#endif
}

} // namespace mtx