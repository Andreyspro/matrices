#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <regex>

#include "boost/asio.hpp"
#include "mtxsolver.h"
#include "mtxaux.h"

// namespace fs = boost::filesystem;


size_t verbosity = 2;

void client_works(const std::string &mtx_data_file_path) {
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
		mtx.LoadFromFile(mtx_data_file_path);
		write(sock, buffer(std::string(MTX_NET_HEADER_HELLO) + "\n"));
		write(sock, buffer(std::string(MTX_NET_CMD_RECEIVE_MTX_AND_CALC) + "\n"));
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

int main(int argc, char *argv[])
{
	std::string mtx_data_file;
	if (argc >= 2)
	{
		mtx_data_file = argv[1];
	}
	else
	{
		std::cout << "Error. argument 1 is empty.\n";
		exit(1);
	}
	std::cout << "Main start... \n";
	client_works(mtx_data_file);
	std::cout << "Main end. \n";
	std::cin.get();

	return 0;
}