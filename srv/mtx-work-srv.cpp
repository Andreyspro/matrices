#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

#include "boost/asio.hpp"
#include "mtxsolver.h"
#include "mtxaux.h"

// namespace fs = boost::filesystem;

size_t verbosity = 2;

void server_works()
{
	using namespace boost::asio;
	std::cout << "Start server !\n";
	try
	{
		std::cout << "version -" << get_hello_version("         MTXSOLVER-HELLO,011.23.459##") << "\n";
		// ip::tcp::endpoint ep(ip::tcp::v4(), mtx_def_ip_port);
		// boost::asio::io_service service;
		// ip::tcp::acceptor acceptor(service, ep);
		// ip::tcp::socket sock(service);
		// std::cout << "Wait for connections...\n";
		// acceptor.accept(sock);
		// std::cout << "Connection ok\n";
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
	server_works();
	std::cout << "Main end. \n";
	std::cin.get();

	return 0;
}