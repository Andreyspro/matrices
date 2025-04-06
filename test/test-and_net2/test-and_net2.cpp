#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <functional>

#include "boost/asio.hpp"
#include "and_net2.h"
#include "uuid.h"
#include "perf_timer.h"

#define NOEXTRAOUT
#define BINARYFILE

namespace net = boost::asio;
using tcp = net::ip::tcp;
const short TCPPORT = 18000;
size_t runcount = 0;
// namespace fs = boost::filesystem;


void client_works4() {
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
	std::ifstream data_file("./data/example_data.txt", std::ios::in);
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
	#endif //ifndef BINARYFILE
	#ifdef BINARYFILE
	std::ifstream data_file("./data/example_data.txt",  std::ios::binary);
	const size_t buff_size = 2048 	;
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
	#endif // BINARYFILE
	sock.close();
	std::cout << "End client !\n";
}

void callback1(std::weak_ptr<std::ofstream> out_f_ptr,
	std::shared_ptr<and_net::net_two> net,
	and_net::op_status_t op_status,
	// std::shared_ptr<std::string> str)
	std::string str)
{
	#ifdef EXTRAOUT
	std::string from = "From callback1 (" + new_uuid() + "):";
	#endif // EXTRAOUT
	#ifdef EXTRAOUT2
	std::string from = "From callback1 (" + new_uuid() + "):";
	std::string status_str = (op_status.read_status == and_net::read_status_t::READ_OK ? "OK" : "Failed"); 
	std::cout << from << "started. status is - " << status_str << " >>>>>>>" << std::endl;
	std::cout << std::endl;
	if (str.get())
		std::cout <<from << "Readed str is - '" << *str << "'" << std::endl;
	std::cout << from << "net ref count=" << net.use_count() << std::endl;
	#endif // EXTRAOUT2

	if (op_status.read_status == and_net::read_status_t::READ_OK) {
		#ifdef EXTRAOUT
		std::cout << from << "str is - '" << *str << "'" << std::endl;
		#endif
		// *(out_f_ptr.lock()) << *str;
		*(out_f_ptr.lock()) << str;
		if (!net->is_EOS()) {
			#ifdef EXTRAOUT2
			std::cout << from << "Start invoke new async_read_str" << std::endl;
			#endif
			net->async_read_str_sz(512,
				std::bind(
					callback1,
					out_f_ptr,
					std::placeholders::_1, 
					std::placeholders::_2, 
					std::placeholders::_3
				)
			);
			#ifdef EXTRAOUT2
			std::cout << from << "End invoke new async_read_str" << std::endl;
			#endif
		}
	}
	else if (op_status.read_status == and_net::read_status_t::READ_FAILED 
		&& op_status.reason == and_net::errors::LAST_MSG_SHORTER)
	{
		std::string last_str =  net->remains_str();
		#ifdef EXTRAOUT
		std::cout << from << "Last msg is - '" << last_str << "'" << std::endl;
		#endif //EXTRAOUT
		*out_f_ptr.lock() << last_str;
	}
	else
	{
		std::cout << "ERROR: read_status =" << op_status.read_status << ", reason" << op_status.reason << "'" << std::endl;
	}
	#ifdef EXTRAOUT
	std::cout << from << "net ref count=" << net.use_count() << std::endl;
	#endif
	#ifdef EXTRAOUT2
	std::cout << from << "Finish <<<<<<" << std::endl << std::endl;
	#endif
}

void server_works4()
{
	using namespace boost::asio;
	std::cout << "Start server !" << std::endl;
	try
	{
		ip::tcp::endpoint ep(ip::tcp::v4(), TCPPORT);
		boost::asio::io_service service;
		ip::tcp::acceptor acceptor(service, ep);
		ip::tcp::socket sock(service);
		std::cout << "Wait for connections..." << std::endl;
		acceptor.accept(sock);

		std::cout << "Connection ok" << std::endl;
		std::shared_ptr<and_net::net_two> Net2(new and_net::net_two(&sock, 8 * 1024));
		std::cout << "From server4: Net ref count=" << Net2.use_count() << std::endl;
		// service.post(std::bind(
		// 	&and_net::net_two::async_read_str_sz, Net2, 10,
		// 	and_net::net_two::on_read_str_f_t(
		// 		std::bind(
		// 			callback1, std::placeholders::_1, std::placeholders::_2, 
		// 			std::placeholders::_3
		// 		)
		// 	)	
		// ));
		std::shared_ptr<std::ofstream> out_file_ptr(new std::ofstream("./data/out.txt", std::ios::out));
		service.post(
			std::bind(
				&and_net::net_two::async_read_str_sz, Net2, 512,
				(and_net::net_two::on_read_str_f_t)
				// (std::function<void(std::shared_ptr<and_net::net_two>, and_net::op_status_t, std::shared_ptr<std::string>)>)
					std::bind(
						callback1,
						out_file_ptr,
						std::placeholders::_1,
						std::placeholders::_2,
						std::placeholders::_3
					)
			)
		);

		
		std::cout << "From server4: post ok" << std::endl;
		std::cout << "From server4: Net ref count=" << Net2.use_count() << std::endl;


		// Net2->async_read_str_sz(
		// 	4,
		// 	std::bind(
		// 		callback1, Net2, /* out_file_ptr, */ std::placeholders::_1, std::placeholders::_2
		// 	)
		// );

		std::cout << "Start service.RUN" << std::endl;
		// std::cout << "file ptr ref count is - " << out_file_ptr.use_count() << std::endl;
		perf_timer<std::chrono::milliseconds> pt;
		service.run();
		pt.stop();
		std::cout << "From server4: run duration - " << pt.get_duration() << " millisec." <<  std::endl;
		// std::cout << "file ptr ref count is - " << out_file_ptr.use_count() << std::endl;
		std::cout << "After service.RUN" << std::endl;
		std::cout << "From server4: Net ref count=" << Net2.use_count() << std::endl;
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
	char start_mode = 2;
	// Check argumets 1 - client, 2 -server
	if (argc >= 2) {
	 try {
		start_mode = std::stoi(std::string(argv[1]));
	 } catch (std::invalid_argument const& ex) {
		std::cout << "Invalid argument !.\n";
		exit(1);
	 }
	}
	if (start_mode == 1)
	{
		std::cout << "Run as client !\n";
		client_works4();
	}
	else if (start_mode == 2)
	{
		std::cout << "Run as server !\n";
		server_works4();
	}
	std::cout << "Main end." << std::endl;
	std::cin.get();

	return 0;
}