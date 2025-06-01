#include <iostream>
#include <iomanip>
// #include <stdexcept>
#include <string>
#include <functional>
#include <thread>
#include <fstream>
#include <boost/filesystem.hpp>

#include "boost/asio.hpp"
#include "mtxsolver.h"
#include "mtxsolver_async.h"
#include "mtx_net_handler.h"
#include "mtxaux.h"
#include "and_net3.h"
#include "perf_timer.h"
#include "uuid.h" 
#include "queuemt.h"

#define EXTRAOUT

namespace net = boost::asio;
using tcp = net::ip::tcp;

size_t save_thread_num = 1;
size_t solve_thread_num = 1;
queuemt<MtxSolver> mtx_q_to_solve(50);
queuemt<MtxSolver> mtx_q_to_save;

std::atomic<size_t> recieved_mtx(0);

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

void client_works3(const size_t buff_size, const std::string &mtx_dir, const std::string ip_addr) {
	using namespace boost::asio;
	namespace fs = boost::filesystem;
	std::cout << "Start client !\n";
	// fs::path mtx_path(mtx_dir);
	if (!fs::exists(mtx_dir))
	{
		std::cout << "Directory  " << mtx_dir << " is not exist." << std::endl;
		exit(1);
	}
	std::vector<fs::path> mtx_paths;
	for (auto const &dir_element : fs::directory_iterator(mtx_dir))
	{
		if (dir_element.is_regular_file() && dir_element.path().extension() == ".mtx")
		{
			// std::cout << "Add '" << dir_element.path().filename() << "'" << std::endl;
			mtx_paths.push_back(dir_element.path());
		}
	}
	std::cout << "Found " << mtx_paths.size() << " .mtx files." << std::endl;
	io_service service;
	ip::tcp::endpoint ep(ip::address::from_string(ip_addr), mtx::def_ip_port);
	size_t num = 1;
	for (auto const &mtx_path : mtx_paths)
	{
		std::cout << "File " << num << " from " << mtx_paths.size() <<  "\n";
		try 
		{
			std::cout << "client binary mode. file - " << mtx_path.filename() <<  "\n"; 
			std::ifstream data_file(mtx_path.c_str(),  std::ios::binary);
			// const size_t buff_size = 2048; 
			// char buff[buff_size];
			std::vector<char> buff(buff_size);
			data_file.seekg(0, std::ios_base::end);
			auto file_size = data_file.tellg();
			size_t to_send_sz;
			size_t byte_remain = file_size;
			data_file.seekg(0, std::ios_base::beg);

			std::cout << "Connecting...\n";
			ip::tcp::socket sock(service);
			sock.connect(ep);
			perf_timer<std::chrono::microseconds> pt1;
			std::cout << "Connecting ok \n";
			write(sock, buffer(std::string(MTX_NET_HEADER_HELLO) + "\n"));
			write(sock, buffer(std::string(MTX_NET_CMD_RECEIVE_MTX_AND_CALC) + "\n"));
			std::cout << "Send matrix \n";

			while (byte_remain)
			{
				to_send_sz = buff_size < byte_remain ? buff_size : byte_remain;
				data_file.read(buff.data(), to_send_sz);
				boost::asio::write(sock, buffer(buff, to_send_sz));
				byte_remain -= to_send_sz;
			}
			pt1.stop();
			double speed_mb_sec = pt1.get_duration() 
				? static_cast<double>(file_size) / static_cast<double>(pt1.get_duration()) * 1000000 / 1024 / 1024
				: 99999999;

			std::cout << "Send matrix " << mtx_path.generic_string() << " in binary mode done. " 
				<< std::setprecision(4) << std::fixed 
				<< static_cast<double>(file_size) / 1024 / 1024 << " MB in " 
				<< static_cast<double>(pt1.get_duration()) / 1000000 << " sec. Speed "
				 << speed_mb_sec << " MB/sec." << std::endl;
			sock.close();
		}
		catch (const std::runtime_error &ex)
		{
			std::cerr << "Error: " << ex.what() << std::endl;
		}
		num++;
	}
	std::cout << "End client !\n";
}	

// predefine
void mtx_load_ready(MtxSolver mtx);

void handle_accept3(net::io_service &service, tcp::acceptor &acceptor, 
	mtx::mtx_net_handler::ptr_t mtx_handler, const boost::system::error_code &err)
{
	#ifdef EXTRAOUT
	std::string prefix = "handle_accept3 (" + new_uuid() + "). ";
	std::cout << prefix << "Start handle mtx " << recieved_mtx.load() << std::endl;
	std::cout << prefix << "Error code - '" << err.value()
	<< "',error message - '" << err.message() << "'" << std::endl;
	#endif
	
	
	#ifdef EXTRAOUT
	std::cout << prefix << "start_handle_connection" << std::endl;
	#endif
	
	mtx_handler->start_handle_connection(mtx_load_ready);

	#ifdef EXTRAOUT
	std::cout << prefix << "start_handle_connection started" << std::endl;
	#endif

	mtx::mtx_net_handler::ptr_t new_mtx_handler = mtx::mtx_net_handler::new_instance(service);
	
	#ifdef EXTRAOUT
	std::cout << prefix << "Start async_accept from " << std::endl;
	#endif
	// new_mtx_handler->get_socket().remote_endpoint().address().to_string()
	acceptor.async_accept
	(
		new_mtx_handler->get_socket(),
		std::bind
		(
			handle_accept3,
			std::ref(service),
			std::ref(acceptor),
			new_mtx_handler,
			std::placeholders::_1
		)
	);
	
	#ifdef EXTRAOUT
	std::cout << prefix << "End." << std::endl;
	#endif

}


void mtx_load_ready(MtxSolver mtx)
{
	#ifdef EXTRAOUT
	auto name = mtx.get_name();
	std::cout << "Add matrix for solve -> " << name << std::endl;
	#endif
	mtx_q_to_solve.wait_and_push(std::move(mtx));
	recieved_mtx++;
	#ifdef EXTRAOUT
	std::cout << "Adding matrix complete -> " << name << std::endl;
	#endif
}

void solve_mtx(queuemt<MtxSolver> &q_to_solve, queuemt<MtxSolver> &q_to_save)
{
	MtxSolver temp_mtx;
	#ifdef EXTRAOUT
	std::cout << "solve_mtx: wait_and_pop." << std::endl;
	#endif
	while(q_to_solve.wait_and_pop(temp_mtx))
	{
		#ifdef EXTRAOUT
		std::cout << "solve_mtx: mtx is dequeued -> " << temp_mtx.get_name() << ". " << "Start solve "<< std::endl;
		std::cout << ">>>>> solve queue size is " << q_to_solve.size() << std::endl;
		#endif
		perf_timer<std::chrono::milliseconds> pt1;
		temp_mtx.Solve();
		pt1.stop();
		#ifdef EXTRAOUT
		std::cout << "solve_mtx: mtx is solved -> " << temp_mtx.get_name() << ". Solve time is " << pt1.get_duration()
			<< " mSec. Dispath mtx for save answers." << std::endl;
		#endif
		temp_mtx.free();
		auto name = temp_mtx.get_name();
		q_to_save.wait_and_push(std::move(temp_mtx));
		#ifdef EXTRAOUT
		std::cout << "solve_mtx: mtx is dispathced to save -> " << name << std::endl;
		#endif
	}
}

void save_mtx(queuemt<MtxSolver> &q_to_save, std::string directory_to_save)
{
	MtxSolver temp_mtx;
	#ifdef EXTRAOUT
	std::cout << "save_mtx: wait_and_pop." << std::endl;
	#endif
	while(q_to_save.wait_and_pop(temp_mtx))
	{
		#ifdef EXTRAOUT
		std::cout << "save_mtx: mtx is dequeued -> " << temp_mtx.get_name() << ". " << "Start save "<< std::endl;
		std::cout << "save queue size is " << q_to_save.size() << std::endl;
		#endif
		boost::filesystem::path save_path;
		save_path /= directory_to_save;
		save_path /= temp_mtx.get_name() + ".ans";
		temp_mtx.SaveAnswers(save_path.string());
		#ifdef EXTRAOUT
		std::cout << "save_mtx: mtx is saved -> " << temp_mtx.get_name() << std::endl;
		#endif
	}
}

void server_works3(size_t buff_size, std::string directory_to_save)
{
	std::vector<std::thread> mtx_solve_threads;
	std::vector<std::thread> mtx_save_threads;

	size_t solve_thread_num = std::thread::hardware_concurrency() >= 2 ? std::thread::hardware_concurrency() - 1 : 1;
	for (size_t i = 0; i < solve_thread_num; i++)
	{
		mtx_solve_threads.push_back
		(
			std::thread(solve_mtx, std::ref(mtx_q_to_solve), std::ref(mtx_q_to_save))
		);
	}
	for (size_t i = 0; i < save_thread_num; i++)
	{
		mtx_save_threads.push_back
		(
			std::thread(save_mtx, std::ref(mtx_q_to_save), directory_to_save)
		);
	}

	net::ip::tcp::endpoint ep(tcp::v4(), mtx::def_ip_port);
	net::io_service service;
	tcp::acceptor acceptor(service, ep);
	{
	mtx::mtx_net_handler::ptr_t mtx_handler =  mtx::mtx_net_handler::new_instance(service);
	#ifdef EXTRAOUT
	std::cout << "server_works3. Wait for connection" << std::endl;
	#endif
	acceptor.async_accept
	(
		mtx_handler->get_socket(),
		std::bind
		(
			handle_accept3,
			std::ref(service),
			std::ref(acceptor),
			mtx_handler,
			std::placeholders::_1
		)
	);
	}
	#ifdef EXTRAOUT
	// std::cout << "server_works3. mtx_handler.use_count() " << mtx_handler.use_count() << std::endl;
	std::cout << "server_works3. Start service RUN()" << std::endl;
	#endif
	service.run();
	#ifdef EXTRAOUT
	std::cout << "server_works3. End" << std::endl;
	#endif

}


int main(int argc, char *argv[])
{
	std::cout << "Main start... \n";
	// std::string s("312.123");
	// double d = crack_atof::atof(&s.front(), &s.back());
	// std::cout << std::setprecision(4) <<  std::fixed <<  d << "\n";
	const size_t buff_size_kb = 8;
	// Argument 1 - client
	// Argument 2 - server
	std::string start_mode = ""; //default start_mode is "" - nothing
	start_mode = argc >= 2 ? argv[1] : "" ;
	
	// Check second agrument net mtx_name
	//For client its directory for load matrix for send to server
	//For server its directory for save answers
	std::string mtx_path = "";
	mtx_path = argc >= 3 ? argv[2] : "" ;
	
	// Check 3 agrument ip addres for connetion
	std::string ip_addr = "";
	ip_addr = argc >= 4 ? argv[3] : "" ;
	
	
	if (start_mode == "client")
	{
		if (mtx_path == "")
		{
			std::cout << "Path for mtx files (arg4) is not specified.\n";
			exit(1);
		}	
		if (ip_addr == "")
		{
			std::cout << "IP addr of server (arg4) not specified.\n";
			exit(1);
		}	
		std::cout << "Run as client!\n" <<
			"path for mtx file = '" << mtx_path << "'\n" <<
			"Buff size = " << buff_size_kb << "kb.\n" <<
			"Server IP = " << ip_addr << "\n";
		client_works3(buff_size_kb * 1024, mtx_path, ip_addr);
	}
	else if (start_mode == "server")
	{
		std::cout << "Run as server!\n" <<
			"path for answers file = '" << mtx_path << "'\n" <<
			"Buff size = " << buff_size_kb << "kb.\n";
		server_works3(buff_size_kb * 1024, mtx_path);
	}
	else
	{
		std::cout << "Start mode (arg1) not specified or incorrect.\n";
	}

	std::cout << "Main end. \n";
	// std::cin.get();

	return 0;
}