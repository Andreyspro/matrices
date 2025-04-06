#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

#include "boost/asio.hpp"
#include "mtxsolver.h"
#include "mtxaux.h"
// #include "and_net.h"
#include "and_net3.h"

#define EXTRAOUT

namespace net = boost::asio;
using tcp = net::ip::tcp;

// namespace fs = boost::filesystem;

size_t verbosity = 2;

void handle_connection(tcp::socket &sock, const std::string &answer_file_path)
{
	// and_net::net_one net1(&sock);
	and_net::net_three::ptr_t net1 = and_net::net_three::get_new(&sock, 1024);
	mtx::version_t client_hello_version = mtx::parse_net_hello(net1->read_str("\n"));
	if (client_hello_version > mtx::MTX_NET_SUPPORTED_VER)
	{
		throw (std::runtime_error("Client version not supported"));
	}
	std::string command = net1->read_str("\n");
	if (command == MTX_NET_CMD_RECEIVE_MTX_AND_CALC) {
		MtxSolver mtx;
		#ifdef EXTRAOUT
			std::cout << "Start LoadFromNet\n";
		#endif
		mtx.LoadFromNet(net1);
		#ifdef EXTRAOUT
			std::cout << "End LoadFromNet\n";
		#endif
		#ifdef EXTRAOUT
			std::cout << "Start solving\n";
		#endif
		mtx.Solve();
		#ifdef EXTRAOUT
			std::cout << "End solving\n";
			std::cout << "Start save\n";
		#endif
		mtx.SaveAnswers(answer_file_path);
		#ifdef EXTRAOUT
			std::cout << "End save\n";
		#endif
	} else {
		throw (std::runtime_error("Unknow command from client"));
	}
}

void server_works(const std::string &answer_file_path)
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
		try
		{
			handle_connection(sock, answer_file_path);
		}
		catch (std::runtime_error const &ex)
		{
			std::cout << "Error: " << ex.what() << "\n";
		}
		std::cout << "Handle connection done\n";




		// size_t bytes;
		// net::streambuf read_buf;
		// std::istream read_stream(&read_buf);
		// std::string read_str;

		// bytes = read(sock, read_buf,  net::transfer_exactly(MTX_HELLO_SIZE));

		// std::getline(read_stream, read_str);
		// bytes = net::read(sock, read_buf,  net::transfer_exactly(MTX_HELLO_SIZE));
		// std::getline(read_stream, read_str);
		// std::cout << "'" << read_str << "'\n";

		// bytes = read(sock, read_buf,  transfer_exactly(MTX_HELLO_SIZE));
		// std::cout << "Read hello  ok\n";
		// std::getline(read_stream, read_str);
		// std::cout << "hello_str is: \'" << read_str << "\'\n";
		// mtx_version_t hello_ver = get_hello_version(read_str);
		// if ( hello_ver >= 0 && hello_ver <= mtx_curr_suppoted_ver) {
		// 	std::cout << "Negotiation OK" << std::endl;
		// } else {
		// 	std::cout << "Negotiation failed\n";
		// }


		// read_buf.commit()
		// bytes = read(sock, read_buf,  transfer_exactly(MTX 
		// std::cout << "CMD is : \'" << read_str << "\'\n";
		// if (read_str == MTX_CMD_RECEIVE_MTX_AND_CALC)
		// {
		// 	std::cout << "Start load matrix.\n";

			
		// } else 
		// {
		// 	std::cout << "Receive unknown command.\n";
		// }




		// size_t i = 0;
		// std::cout << std::fixed;

		// const char delim = '\n';
		// const size_t max_buff_size = 1024;
		// const size_t full_buff_size = max_buff_size * 2;
		// char *buff = new char[full_buff_size + 1];
		// char *insert_ptr = buff + max_buff_size;
		// char *view_ptr, *delim_ptr, *end_ptr;
		// std::string extracted_str;
		// double d = 0;
		// size_t bytes = 0;
		// size_t msg_size = 0;
		// size_t tail_size = 0;
		// std::ofstream save_file("data/mtx.mtx", std::ios::out);
		// save_file << std::fixed;
		// while(true) 
		// {
		// 	std::this_thread::sleep_for(std::chrono::microseconds(50));
		// 	if ( ( bytes = sock.read_some(buffer(insert_ptr, max_buff_size)) ) == 0)
		// 		continue; //miss this iteration
		// 	++i;
		
		// 	// std::cout << "\n\n\n---------------------------------------------------" << std::endl;
		// 	if (bytes != max_buff_size) 
		// 		std::cout << "Read(" << i << ") " << bytes << " bites" << std::endl;
		// 	else
		// 	{
		// 		if (i % 1000 == 0)
		// 			std::cout << "Read(" << i << ")" << std::endl;
		// 	}
		// 	// std::cout << ">>>\n" << std::string(insert_ptr, bytes) << "\n<<<" << std::endl;
		// 	// std::cout << "---------------------------------------------------" << std::endl;

		// 	// start split part of message
		// 	insert_ptr[bytes] = '\0'; //set end of string marker
		// 	end_ptr = insert_ptr + bytes - 1;
		// 	view_ptr = insert_ptr - tail_size;
		// 	while ((delim_ptr = strchr(view_ptr, delim)) != nullptr) //point to delimeter char
		// 	{
		// 		if ((msg_size = delim_ptr - view_ptr) > 0) {
		// 			extracted_str = std::string(view_ptr, msg_size);
		// 			try
		// 			{
		// 				d = std::stod(extracted_str);
		// 			}
		// 			catch(std::invalid_argument const& ex)
		// 			{
		// 				std::cerr << "Error while convertion \"" << extracted_str << "\" to double -" << ex.what() << '\n';
		// 			}
		// 			// std::cout << i << ". str=\"" << extracted_str  << "\", double=\"" << d <<"\"" << std::endl;
		// 			save_file << d << "\n";
		// 		}
		// 		// if (extracted_str.compare("027.002096") == 0)
		// 		// 	int dummy =0;
		// 		// std::cout << "view_ptr=" << (view_ptr - insert_ptr) << "\" end_ptr=\"" << (end_ptr - insert_ptr) << "\" delim_ptr=\"" 
		// 			// << (delim_ptr - insert_ptr) << "\"" << std::endl;
		// 		view_ptr = delim_ptr + 1; //new position for search
		// 	}
		// 	if (view_ptr == insert_ptr - tail_size) {
		// 		throw std::runtime_error("Can't parse message");
		// 	}
		// 	tail_size = 0;
		// 	if (view_ptr <= end_ptr) {
		// 		tail_size = end_ptr - view_ptr + 1;
		// 		strncpy(insert_ptr - tail_size, view_ptr, tail_size);
		// 	}
		// }
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

	std::string answer_file_path = "";
	if (argc >= 2) {
		answer_file_path = argv[1];
	}
	else
	{
		std::cout << "Error. No argument answer file name..\n";
		exit(1);
	}
	std::cout << "Answer file name - " << answer_file_path << "\n";
	server_works(answer_file_path);
	std::cout << "Main end. \n";
	std::cin.get();

	return 0;
}