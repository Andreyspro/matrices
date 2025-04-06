#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <functional>

#include "boost/asio.hpp"
#include "and_net3.h"
// #include "uuid.h"
#include "perf_timer.h"

#define NOEXTRAOUT
#define BINARYFILE

namespace net = boost::asio;
using tcp = net::ip::tcp;
const short TCPPORT = 18000;
size_t runcount = 0;
// namespace fs = boost::filesystem;


void client_works4(const std::string &data_file_path) {
	using namespace boost::asio;
	// size_t bytes;
	std::cout << "Start client !\n";
	std::cout << "Press enter to continue...!\n";
	std::cin.get();
	io_service service;
	ip::tcp::endpoint ep(ip::address::from_string("127.0.0.1"), TCPPORT);
	std::cout << "Connecting...\n";
	ip::tcp::socket sock(service);
	std::cout << "Connecting ok \n";
	sock.connect(ep);
	std::cout << "Start send data \n";
	#ifndef BINARYFILE
	std::ifstream data_file(data_file_path, std::ios::in);
	std::string buff;
	// std::getline(data_file, buff);
	// std::cout << buff << "\n";
	#ifdef EXTRAOUT
	int line = 1;
	#endif
	while (std::getline(data_file, buff))
	{
		#ifdef EXTRAOUT
		std::cout << line++ << "." << buff << "\n";
		#endif
		sock.write_some(buffer(buff + "\r\n"));
	}
	#endif
	#ifdef BINARYFILE
	std::ifstream data_file(data_file_path,  std::ios::binary);
	const size_t buff_size = 2048;
	char buff[buff_size];
	data_file.seekg(0, std::ios_base::end);
	auto file_size = data_file.tellg();
	// std::cout <<  "File size is - " << file_size << "\n";
	size_t to_send_sz;
	size_t byte_remain = file_size;
	data_file.seekg(0, std::ios_base::beg);
	while (byte_remain)
	{
		to_send_sz = buff_size < byte_remain ? buff_size : byte_remain;
		data_file.read(buff, to_send_sz);
		boost::asio::write(sock, buffer(buff, to_send_sz));
		byte_remain -= to_send_sz;
	}
	#endif
	sock.close();
	std::cout << "End client !\n";
}

void callback1(std::weak_ptr<std::ofstream> out_file_ptr, std::shared_ptr<and_net::net_three> net, 
	and_net::op_fill_status_t fill_status)
{
	#ifdef EXTRAOUT
	std::cout << "+" << net.use_count() << std::endl;
	#endif
	if (fill_status.fill_result == and_net::FILL_FAILED)
	{
			std::cout << "Fill data failed." << std::endl;
			return;
	}

	std::string str;

	and_net::op_read_status_t read_status;
	while (read_status.read_result == and_net::READ_OK)
	{
		// read_status = net->try_read_str_sz(str, 256);
		read_status = net->try_read_str_em(str, "\r\n");
		if (read_status.read_result == and_net::READ_OK)
		{
			#ifdef EXTRAOUT
			std::cout << ">" << str << std::endl;
			#endif // ifdef EXTRAOUT
			*(out_file_ptr.lock()) << str << "\r\n";
		}
	}

	if (read_status.reason == and_net::NEED_MORE_DATA)
	{
		runcount++;
		net->async_fill_buff
		(
			std::bind
			(
				callback1,
				out_file_ptr,
				net->get_ptr(),
				std::placeholders::_1
				// std::placeholders::_1,
				// std::placeholders::_2
			)
		);
		return;
	}
	else if (read_status.reason == and_net::LAST_MSG_SHORTER)
	{
		str = net->remains_str();
		#ifdef EXTRAOUT
		std::cout << "Last msg:>" << str << std::endl;
		#endif
		*(out_file_ptr.lock()) << str ;
	}
	else
	{
		std::cout << "Read error." << str << std::endl;
	}
}


void server_works4(const std::string &out_data_file_path, size_t buff_size_kb)
{
	using namespace boost::asio;
	std::cout << "Start server !" << std::endl;
	try
	{
		ip::tcp::endpoint ep(ip::tcp::v4(), TCPPORT);
		boost::asio::io_service service;
		ip::tcp::acceptor acceptor(service, ep);
		ip::tcp::socket sock(service);
		std::shared_ptr<std::ofstream> out_file_ptr(new std::ofstream(out_data_file_path, std::ios::out));
		std::cout << "Wait for connections..." << std::endl;
		acceptor.accept(sock);

		std::cout << "Connection ok" << std::endl;
		// and_net::net_three net3(&sock, 32);
		// std::shared_ptr<and_net::net_three> net3(new and_net::net_three(&sock, buff_size_kb * 1024));
		std::shared_ptr<and_net::net_three> net3 = and_net::net_three::get_new(&sock, buff_size_kb * 1024);
		and_net::op_fill_status_t op_fill;
		std::cout << "Main: Start callback" << std::endl;
		perf_timer<std::chrono::milliseconds> pt;
		callback1(out_file_ptr, net3, op_fill);
		// net3->async_fill_buff()
		service.run();
		// net3->cleancallback();
		pt.stop();
		std::cout << "From server4: run duration - " << pt.get_duration() << " millisec." <<  std::endl;
		std::cout << "From server4: runcount - " << runcount <<  std::endl;
		std::cout << "Main: service.run() finish" << std::endl;
		std::cout << "+" << net3.use_count() << std::endl;
		
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	
	std::cout << "End server !\n";
}

int main(int argc, char *argv[])
{
	std::cout << "Main start... " << std::endl;
	int start_mode = 0;
	std::string data_file_path = "";
	size_t buff_size_kb = 8;

	// Check argumets 1 - client, 2 -server
	if (argc >= 2) {
		try
		{
			start_mode = std::stoi(std::string(argv[1]));
		}
		catch (std::invalid_argument const &ex)
		{
			std::cout << "Invalid argument RUN type!.\n";
			exit(1);
		}
	}

	// Check second arg - data file path
	if (argc >= 3) {
		data_file_path = argv[2];
	}
	
	std::cout << "Start_mode(1) - " << start_mode
		<< "\ndata_file(2) - `" << data_file_path << "` !.\n";

	if (start_mode == 0 || data_file_path == "")
	{
		std::cout << "Error argument!\n";
		exit(1);
	} 

	// Check thrid agrument - buffer size
	if (argc >= 4) {
		try
		{
			buff_size_kb = std::stoi(std::string(argv[3]));
		}
		catch (std::invalid_argument const &ex)
		{
			std::cout << "Invalid argument buffer size!\n";
			exit(1);
		}
	}
	
	if (start_mode == 1)
	{
		std::cout << "Run as client !\n";
		client_works4(data_file_path);
	}
	else if (start_mode == 2)
	{
		std::cout << "Run as server! Buff size = " << buff_size_kb << "kb.\n";
		server_works4(data_file_path, buff_size_kb);
	}

	std::cout << "Main end." << std::endl;
	std::cin.get();

	return 0;
}