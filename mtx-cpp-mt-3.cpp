#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>

#include "boost/filesystem.hpp"
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "mtxsolver.h"
#include "queuemt.h"
#include "perf_timer.h"

namespace fs = boost::filesystem;
std::string mtxPath{"/home/andreys/mtxs/"};
std::atomic<bool> mtx_is_loaded;
std::atomic<unsigned> mtx_count(0);
std::atomic<unsigned> finded_mtx(0);
std::atomic<unsigned> loaded_mtx(0);
std::atomic<unsigned> calculated_mtx(0);
std::atomic<unsigned> full_queue_condition(0);
std::atomic<unsigned> load_time(0);
std::atomic<unsigned> calc_time(0);
std::atomic<unsigned> prefetched_mtx_max_count(0);

size_t max_load_queue_size = 0;
size_t WAIT_TIME = 25;
size_t verbosity = 2;

bool loadMatrix(queuemt<fs::path>& MtxQueueFiles, queuemt<MtxSolver>& MtxQueueToSolve)
{
	std::string MtxFileName;
	fs::path path;
	size_t count = 0;

	std::cout << std::this_thread::get_id() << ". Start LOAD thread.\n";
	while (MtxQueueFiles.try_pop(path)) 
	{
		MtxSolver Mtx;
		++count;
		if (verbosity >= 2)
			std::cout << std::this_thread::get_id() 
				<< ". Start load matrix #" << count << "  -> " << path.filename().string() << " < ++++++ \n";
		perf_timer<std::chrono::milliseconds> load_timer1;
		try {
			Mtx.LoadFromFile(path.string());
		} catch (std::runtime_error& ex) {
			std::cout << "Error - " << ex.what() << "\n";
			exit(1);
		}

		load_timer1.stop();
		++loaded_mtx;
		load_time += load_timer1.get_duration();
		if (verbosity >= 2) {
			std::cout << std::this_thread::get_id() << "End load. Load time \""
				<< path.filename().string()	<< "\" - "
				<< load_timer1 << " milliseconds.\n";
		}

		// while (!MtxQueueToSolve.try_push(std::move(Mtx)))
		// {
		// 	if (verbosity >= 1)
		// 		std::cout << std::this_thread::get_id() << ". Queue to solve is full, wait and retry...\n";
		// 	++full_queue_condition;
		// 	std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
		// }
		if (!MtxQueueToSolve.wait_and_push(std::move(Mtx))) {
			std::cout << std::this_thread::get_id() << ". Error while put " << path.filename().string() << " in the queuemt. \n";
		}
		if (verbosity >= 1)
			std::cout << "Loader. Preload MTX size - " << MtxQueueToSolve.size() << "\n";
		if (verbosity >= 2)
			std::cout << std::this_thread::get_id() << ". End put " << path.filename().string() << " in the queuemt. \n";
	}
	std::cout << std::this_thread::get_id() << ". Load " << count << " matrices.\n";
	return true;
}

bool calcMatrix(queuemt<MtxSolver>& MtxQueueToSolve, queuemt<MtxSolver>& MtxQueueSolved) 
{
	MtxSolver temp_mtx;
	std::cout << std::this_thread::get_id() << ". Start CALC thread.\n";
	while (MtxQueueToSolve.wait_and_pop(temp_mtx))
	{
		if (verbosity >= 1)
			std::cout << "Calc. Preload MTX size - " << MtxQueueToSolve.size() << "\n";

		if (verbosity >= 2)
			std::cout << std::this_thread::get_id() << ". Start calc " << temp_mtx << "...\n";
		perf_timer<std::chrono::milliseconds> calc_timer1;
		temp_mtx.Solve();
		calc_timer1.stop();
		calc_time += calc_timer1.get_duration();
		temp_mtx.free();
		++calculated_mtx;
		if (verbosity >= 2) 
		{
			std::cout << std::this_thread::get_id() << "End calc. Calc time \""	<< temp_mtx << "\" - "
				<< calc_timer1 << " milliseconds.\n";
		}

		MtxQueueSolved.try_push(std::move(temp_mtx));

	}
		
		// if (MtxQueueToSolve.try_pop(temp_mtx))
		// {
		// 	if (verbosity >= 2)
		// 		std::cout << std::this_thread::get_id() << ". Start calc " << temp_mtx << " <++++++++++++++++++++\n";
		// 	auto beginTime = std::chrono::steady_clock::now();
		// 	temp_mtx.Solve();
		// 	temp_mtx.free();
		// 	auto endTime = std::chrono::steady_clock::now();
		// 	auto workTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime);
		// 	if (verbosity >= 2) 
		// 	{
		// 		std::cout << "Calc time \""
		// 		<< temp_mtx
		// 		<< "\" - "
		// 		<< std::fixed << std::setprecision(2)
		// 		<< workTime.count()
		// 		<< " milliseconds.\n";
		// 		std::cout << std::this_thread::get_id() << ". End calc " << temp_mtx << "\n";
		// 	}
		// 	MtxQueueSolved.try_push(std::move(temp_mtx));
			
		// } 
		// else 
		// {
		// 	// std::cout << std::this_thread::get_id() 
		// 	// 		<< ". Wait for new loaded matrix \n";
		// 	std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
		// }
	
	std::cout << std::this_thread::get_id() << ". End CALC thread.\n";
	return true;
}

int main(int argc, char *argv[])
{
	size_t MAX_PREFETCH_MTX = 6;
	size_t MAX_LOAD_THREADS = 2;
	size_t MAX_CALC_THREADS = 4;
	std::cout << "Main start... \n";
	if (argc == 6) {
		try
		{
			MAX_PREFETCH_MTX = std::stoi(std::string(argv[1]));
			MAX_LOAD_THREADS = std::stoi(std::string(argv[2]));
			MAX_CALC_THREADS = std::stoi(std::string(argv[3]));
			WAIT_TIME = std::stoi(std::string(argv[4]));
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
	std::cout << "WAIT_TIME - " << WAIT_TIME <<"\n";
	std::cout << "verbosity - " << verbosity <<"\n";

	#ifdef EXTRAOUT
		std::cout << "Debug mode is ON \n";
	#endif

	// RUN UID
	boost::uuids::random_generator gen;
	std::string runUID = boost::uuids::to_string(gen()).substr(0, 6);

	// Fill list of matrices files.
	std::cout << "Start search files to load. ========================================\n";
	queuemt<fs::path> MtxFilesList;
	for (auto const &dirPath : fs::directory_iterator(mtxPath))
	{
		if (!fs::is_directory(dirPath)) {
			MtxFilesList.try_push(dirPath.path());
			++finded_mtx;
		}
	}
	std::cout << "Search files is done. ========================================\n\n";

	std::cout << "Start load and Calculation. ========================================\n";

	// Load Mtx form files one by one with limited size
	mtx_is_loaded.store(false);
	queuemt<MtxSolver> mtx_queue_to_solve(MAX_PREFETCH_MTX);
	queuemt<MtxSolver> mtx_queue_solved;
	std::vector<std::thread> loadThreads;
	std::vector<std::thread> calcThreads;
	perf_timer<std::chrono::milliseconds> timer1;
	// Start load threads.
	for (size_t i = 0; i < MAX_LOAD_THREADS; i++)
	{
		loadThreads.push_back(std::thread(loadMatrix, std::ref(MtxFilesList), 
			std::ref(mtx_queue_to_solve)));
	}

	// Start calc threads.
	for (size_t i = 0; i < MAX_CALC_THREADS; i++)
	{
		calcThreads.push_back(std::thread(calcMatrix, std::ref(mtx_queue_to_solve), 
			std::ref(mtx_queue_solved)));
	}
	
	for (auto &loadThread : loadThreads)
	{
		loadThread.join();
	}
	mtx_is_loaded.store(true);
	mtx_queue_to_solve.stop_service();
	for (auto &calcThread : calcThreads)
	{
		calcThread.join();
	}
	
	timer1.stop();
	std::cout << "Load + Calc time - " << std::fixed << std::setprecision(2)
		<< timer1 << " milliseconds.\n";

	std::cout << "To solve queue size -" << mtx_queue_to_solve.size() << "\n";
	std::cout << "Solved matrices size -" << mtx_queue_solved.size() << "\n";
	std::cout << "Finded/lodaded/calulated matrices -" << finded_mtx << "/" 
		<< loaded_mtx << "/" << calculated_mtx << "\n";
	std::cout << "Full queue condition -" << full_queue_condition << "\n";
	std::cout << "Load time - " << load_time << "\n";
	std::cout << "Calc time -" << calc_time << "\n";

	std::cout << "Main end. \n";
	std::cin.get();

	return 0;
}