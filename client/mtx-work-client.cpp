#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <regex>

#include "boost/asio.hpp"
#include "mtxsolver.h"
#include "mtxvardefines.h"

// namespace fs = boost::filesystem;

size_t verbosity = 2;

void client_works() {
	using namespace boost::asio;
	std::cout << "Start client !\n";
	io_service service;
	ip::tcp::endpoint ep(ip::address::from_string("127.0.0.1"), mtx_def_ip_port);
	std::cout << "Connecting...\n";
	ip::tcp::socket sock(service);
	std::cout << "Connecting ok \n";
	sock.connect(ep);
	sock.close();
	std::cout << "End client !\n";
}	

int main(int argc, char *argv[])
{
	std::cout << "Main start... \n";
	client_works();
	std::cout << "Main end. \n";
	std::cin.get();

	return 0;
}