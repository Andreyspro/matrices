// #include <iomanip>
#include <iostream>
// #include <stdexcept>
#include <string>
#include <functional>
#include <thread>
#include <fstream>

#include "boost/asio.hpp"
#include "mtxsolver.h"
#include "mtxsolver_async.h"
#include "mtxaux.h"
#include "and_net3.h"
#include "perf_timer.h"

#define noEXTRAOUT

namespace net = boost::asio;
using tcp = net::ip::tcp;

// namespace fs = boost::filesystem;

size_t verbosity = 2;

void client_works(const std::string &mtx_file_name) {
	using namespace boost::asio;
	std::cout << "Start client !\n";
	try {
		io_service service;
		ip::tcp::endpoint ep(ip::address::from_string("127.0.0.1"), mtx::def_ip_port);
		std::cout << "Connecting...\n";
		ip::tcp::socket sock(service);
		sock.connect(ep);
		std::cout << "Connecting ok \n";
		MtxSolver mtx;
		mtx.LoadFromFile(mtx_file_name);
		write(sock, buffer(std::string(MTX_NET_HEADER_HELLO) + "\r\n"));
		std::this_thread::sleep_for(std::chrono::seconds(1));
		write(sock, buffer(std::string(MTX_NET_CMD_RECEIVE_MTX_AND_CALC) + "\r\n"));
		std::cout << "Send matrix \n";
		mtx.SendToNet(sock);
		std::cout << "Send matrix done \n";
		sock.close();
	}
	catch (const std::runtime_error &ex)
	{
		std::cerr << "Error: " << ex.what() << std::endl;
	}
	std::cout << "End client !\n";
}	

void client_works2(const std::string &mtx_file_name) {
	using namespace boost::asio;
	std::cout << "Start client !\n";
	try {
		std::cout << "client binary mode. file - " << mtx_file_name <<  "\n";
		std::ifstream data_file(mtx_file_name,  std::ios::binary);
		const size_t buff_size = 2048 	;
		char buff[buff_size];
		data_file.seekg(0, std::ios_base::end);
		auto file_size = data_file.tellg();
		size_t to_send_sz;
		size_t byte_remain = file_size;
		data_file.seekg(0, std::ios_base::beg);

		io_service service;
		ip::tcp::endpoint ep(ip::address::from_string("127.0.0.1"), mtx::def_ip_port);
		std::cout << "Connecting...\n";
		ip::tcp::socket sock(service);
		sock.connect(ep);
		std::cout << "Connecting ok \n";
		write(sock, buffer(std::string(MTX_NET_HEADER_HELLO) + "\n"));
		write(sock, buffer(std::string(MTX_NET_CMD_RECEIVE_MTX_AND_CALC) + "\n"));
		std::cout << "Send matrix \n";

		while (byte_remain)
		{
			to_send_sz = buff_size < byte_remain ? buff_size : byte_remain;
			data_file.read(buff, to_send_sz);
			boost::asio::write(sock, buffer(buff, to_send_sz));
			byte_remain -= to_send_sz;
		}

		std::cout << "Send matrix in binary mode done \n";
		sock.close();
	}
	catch (const std::runtime_error &ex)
	{
		std::cerr << "Error: " << ex.what() << std::endl;
	}
	std::cout << "End client !\n";
}	

enum nego_operation_t
{
	READ_HELLO_HEADER = 0,
	READ_COMMAND = 1,
	LOAD_MTX = 2
};

// predeclare function
void on_load_mtx(bool, bool, mtxsolver_async_loader::self_ptr_t);
void do_load_mtx(std::shared_ptr<and_net::net_three>);

void callback_nego(nego_operation_t op, std::shared_ptr<and_net::net_three> net_ptr, and_net::op_fill_status_t fill_status)
{
	std::string read_str;
	and_net::op_read_status_t read_status;
	if (fill_status.fill_result != and_net::FILL_OK)
	{
		std::cout << "callback_nego. FILL ERROR." << std::endl;
		return;
	}

	if (op == READ_HELLO_HEADER)
	{
		read_status = net_ptr->try_read_str_em(read_str, "\n");
		if (read_status.read_result == and_net::READ_OK)
		{
			mtx::version_t mtx_version = mtx::parse_net_hello(read_str);
			if (mtx_version > 0 && mtx_version <= mtx::MTX_NET_SUPPORTED_VER)
			{
				std::cout << "callback_nego. HELLO header is OK." << std::endl;
				op = READ_COMMAND;
			}
			else
			{
				std::cout << "callback_nego. NET HELLO NOT SUPPORTED." << std::endl;
				return;
			}
		}
	}

	if (op == READ_COMMAND)
	{
		read_status = net_ptr->try_read_str_em(read_str, "\n");
		if (read_status.read_result == and_net::READ_OK)
		{
			if (read_str == MTX_NET_CMD_RECEIVE_MTX_AND_CALC)
			{
				op = LOAD_MTX;
				std::cout << "callback_nego. Command recieved - MTX_NET_CMD_RECEIVE_MTX_AND_CALC." << std::endl;
			}
			else
			{
				std::cout << "callback_nego. NET COMMAND NOT SUPPORTED." << std::endl;
				return;
			}
		}
	}


	if (op < LOAD_MTX &&
		read_status.read_result == and_net::READ_FAILED &&
		read_status.reason == and_net::NEED_MORE_DATA
	)
	{
		std::cout << "callback_nego. read more data." << std::endl;
		net_ptr->async_fill_buff
		(
			std::bind
			(
				callback_nego,
				op,
				net_ptr->get_ptr(),
				std::placeholders::_1
			)
		);
		return;
	}

	if (op == LOAD_MTX)
	{

		std::cout << "callback_nego. Do load mtx." << std::endl;
		do_load_mtx(net_ptr->get_ptr());

	}
	else
	{
		std::cout << "callback_nego. nego error." << std::endl;
		return;
	}
	
}

void do_load_mtx(std::shared_ptr<and_net::net_three> net_ptr)
{
	// mtxsolver_async_loader m(net_ptr);
	mtxsolver_async_loader::self_ptr_t mtxloader_ptr = mtxsolver_async_loader::get_new(net_ptr);
	mtxloader_ptr->start(on_load_mtx);
}

void on_load_mtx(bool load_ok, bool mtx_size, mtxsolver_async_loader::self_ptr_t mtxloader_ptr)
{
	std::cout << "on_load_mtx. start calculating." << std::endl;
	// do calc MTX
}

void handle_connection2(net::io_service &service, tcp::socket &sock, size_t buff_size)
{
	std::shared_ptr<and_net::net_three> net_ptr = and_net::net_three::get_new(&sock, buff_size);
	mtxsolver_async_loader::self_ptr_t mtxloader_ptr = mtxsolver_async_loader::get_new(net_ptr);
	service.post
	(
		std::bind
		(
			callback_nego,
			READ_HELLO_HEADER,
			net_ptr->get_ptr(),
			and_net::op_fill_status_t{}
		)
	);
	service.run();
} // handle_connection2()

void server_works(size_t buff_size)
{
	// using namespace boost::asio;

	std::cout << "Start server !\n";
	try
	{
		// std::cout << "version -" << get_hello_version("         MTXSOLVER-HELLO,011.23.459##") << "\n";
		net::ip::tcp::endpoint ep(tcp::v4(), mtx::def_ip_port);
		net::io_service service;
		tcp::acceptor acceptor(service, ep);
		tcp::socket sock(service);
		std::cout << "Wait for connections...\n";
		acceptor.accept(sock);
		std::cout << "Connection ok\n";
		std::cout << "Start handle connection\n";
		perf_timer<std::chrono::milliseconds> pt;
		try
		{
			handle_connection2(service, sock, buff_size);
		}
		catch (std::runtime_error const &ex)
		{
			std::cout << "Error: " << ex.what() << "\n";
		}
		pt.stop();
		std::cout << "From server_works: run duration - " << pt.get_duration() << " millisec." <<  std::endl;
		std::cout << "Handle connection done\n";

	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	
	std::cout << "End server !\n";
}

int main(int argc, char *argv[])
{
	std::cout << "Main start... \n";

	// Check argumets 1 - client, 2 -server
	char start_mode = 0; //default start_mode is 0 - nothing

	size_t net_buff_size_kb = 8; //default 8kb
	std::string mtx_file_path = "";
	if (argc >= 2) {
		try {
			start_mode = std::stoi(std::string(argv[1]));
		} catch (std::invalid_argument const& ex) 
		{
			std::cout << "Invalid argument-1 (start mode) !.\n";
			exit(1);
		}
	}

	// Check thrid agrument net mtx_file_name
	if (argc >= 3) {
		mtx_file_path = argv[2];
	}

	// Check second agrument net buffer size
	if (argc >= 4) {
		try {
			net_buff_size_kb = std::stoi(std::string(argv[3]));
		} catch (std::invalid_argument const& ex)
		{
			std::cout << "Invalid argument-3 net buffer size!\n";
			exit(1);
		}
	}

	if (start_mode == 1 && mtx_file_path == "")
	{
		std::cout << "Client mode. File not specified !\n";
		exit(1);

	}
	
	if (start_mode == 1)
	{
		std::cout << "Run as client !\n";
		client_works2(mtx_file_path);
	}
	else if (start_mode == 2)
	{
		std::cout << "Run as server! Buff size = " << net_buff_size_kb << "kb.\n";
		server_works(net_buff_size_kb * 1024);
	}
	else
	{
		std::cout << "Start mode (arg1) not specified.\n";
	}

	std::cout << "Main end. \n";
	std::cin.get();

	return 0;
}