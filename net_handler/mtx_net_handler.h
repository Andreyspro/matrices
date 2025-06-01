#ifndef MTX_NET_HANDLER
#define MTX_NET_HANDLER

#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <chrono>
#include <functional>

#include "and_net3.h"
#include "mtxsolver.h"
#include "mtxsolver_async.h"
#include "perf_timer.h"

using namespace boost::asio;

#define MTX_NET_HANDLER_EXTRAOUT
#define MTX_NET_HANDLER_LOAD_BENCMARK
// #undef MTX_NET_HANDLER_EXTRAOUT

namespace mtx
{

enum handle_stage_t
{
	STAGE_READ_HELLO = 0,
	STAGE_READ_COMMAND,
	STAGE_LOAD_MTX
};

typedef std::function<void(MtxSolver)> mtx_load_complete_cb_t;

class mtx_net_handler : public std::enable_shared_from_this<mtx_net_handler>
{
	//default, copy constructor is deleted
	mtx_net_handler() = delete;

	mtx_net_handler(const mtx_net_handler &source) = delete;

	// copy assign operator is deleted
	const mtx_net_handler& operator=(const mtx_net_handler&) = delete;

	// hidden construstor.
	mtx_net_handler(io_service &service);

public:
	typedef std::shared_ptr<mtx_net_handler> ptr_t;

	static ptr_t new_instance(io_service &service);

	ip::tcp::socket & get_socket();

	void start_handle_connection(mtx_load_complete_cb_t cb);
	~mtx_net_handler();
protected:
	void read_hello(and_net::op_fill_status_t fill_status);
	void read_command(and_net::op_fill_status_t fill_status);
	void on_load_mtx(bool load_ok, size_t mtx_size, mtxsolver_async_loader::self_ptr_t mtxloader_ptr);
	
	// void handle_accept_connection()
	ip::tcp::socket m_sock;
	handle_stage_t m_stage;
	and_net::net_three::ptr_t m_anet;
	// mtxsolver_async_loader::self_ptr_t m_mtx_loader;
	#ifdef MTX_NET_HANDLER_LOAD_BENCMARK
	perf_timer<std::chrono::milliseconds> bench_timer;
	#endif
	mtx_load_complete_cb_t m_load_complete_cb;
	#ifdef MTX_NET_HANDLER_EXTRAOUT
	std::string m_id;
	#endif
};

} // namespace mtx

#endif // #define MTX_NET_HANDLER