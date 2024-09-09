#include <iomanip>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <thread>

#include "boost/filesystem.hpp"
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "perf_timer.h"
#include "queuemt.h"
#include "mtxworker.h"
#include "mtxsolver.h"

namespace fs = boost::filesystem;

std::string mtxPath{"/home/andreys/mtxs/"};
size_t verbosity = 2;

int main(int argc, char *argv[])
{
	size_t MAX_PREFETCH_MTX = 6;
	size_t MAX_LOAD_THREADS = 2;
	size_t MAX_CALC_THREADS = 4;
	size_t MAX_SAVE_THREADS = 1;
	size_t finded_mtx = 0;
	std::cout << "Main start... \n";
	if (argc == 6) {
		try
		{
			MAX_PREFETCH_MTX = std::stoi(std::string(argv[1]));
			MAX_LOAD_THREADS = std::stoi(std::string(argv[2]));
			MAX_CALC_THREADS = std::stoi(std::string(argv[3]));
			MAX_SAVE_THREADS = std::stoi(std::string(argv[4]));
			verbosity = std::stoi(std::string(argv[5]));
		}
		catch (std::invalid_argument const& ex)
		{
			std::cout << "Error in run arguments: " << ex.what() << '\n';
			exit(1);
		}
	}
	std::cout << "MAX_PREFETCH_MTX - " << MAX_PREFETCH_MTX <<"\n";
	std::cout << "MAX_LOAD_THREADS - " << MAX_LOAD_THREADS <<"\n";
	std::cout << "MAX_CALC_THREADS - " << MAX_CALC_THREADS <<"\n";
	std::cout << "MAX_SAVE_THREADS - " << MAX_SAVE_THREADS <<"\n";
	std::cout << "verbosity - " << verbosity <<"\n";

	#ifdef EXTRAOUT
		std::cout << "Debug mode is ON \n";
	#endif

	// RUN UID
	boost::uuids::random_generator gen;
	std::string runUID = boost::uuids::to_string(gen()).substr(0, 6);


	queuemt<mtx_source_t> mtx_source_list;
	queuemt<MtxSolver> mtx_to_solve_list(MAX_PREFETCH_MTX);
	queuemt<MtxSolver> mtx_solved_list;

	std::vector<std::thread> mtx_load_threads;
	for (size_t i = 0; i < MAX_LOAD_THREADS; i++)
	{
		mtx_load_threads.push_back(
			std::thread(
				load_mtxs, 
				std::ref(mtx_source_list), 
				std::ref(mtx_to_solve_list)
			)
		);
	}
	
	std::vector<std::thread> mtx_solve_threads;
	for (size_t i = 0; i < MAX_CALC_THREADS; i++)
	{
		mtx_solve_threads.push_back(
			std::thread(
				solve_mtx,
				std::ref(mtx_to_solve_list),
				std::ref(mtx_solved_list)
			)
		);
	}

	std::vector<std::thread> mtx_save_threads;
	for (size_t i = 0; i < MAX_SAVE_THREADS; i++)
	{
		mtx_save_threads.push_back(
			std::thread(
				save_answers,
				std::ref(mtx_solved_list),
				std::string("./answers/")
			)
		);
	}

	std::thread status_thread(print_status);

	// Fill source of matrices.
	std::cout << "Start search files to load. ========================================\n";
	mtx_source_t mtx_source;
	for (auto const &mtx_path : fs::directory_iterator(mtxPath))
	{
		if (!fs::is_directory(mtx_path)) {
			++finded_mtx;
			mtx_source.type = MTX_FILE;
			mtx_source.name = mtx_path.path().filename().string();
			mtx_source.file_name = mtx_path.path().string();
			mtx_source_list.wait_and_push(std::move(mtx_source));
		}
	}
	std::cout << "Search files is done. Find " << finded_mtx << " matrices. ========================================\n\n";

	for (auto &mtx_load_thread : mtx_load_threads) 
	{
		mtx_load_thread.join();
	}
	for (auto &mtx_solve_thread : mtx_solve_threads) 
	{
		mtx_solve_thread.join();
	}
	for (auto &mtx_save_thread : mtx_save_threads) 
	{
		mtx_save_thread.join();
	}

	status_thread.join();
	std::cout << "Main end. \n";
	std::cin.get();

	return 0;
}