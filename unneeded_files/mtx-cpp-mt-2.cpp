
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#include <chrono>

#include "boost/filesystem.hpp"
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "mtxsolver.h"
#include "queuemt.h"
#include <atomic>

namespace fs = boost::filesystem;
const unsigned int MAX_PREFETCH_MTX = 1;
const unsigned int MAX_LOAD_THREADS = 1;
const unsigned int MAX_CALC_THREADS = 4;
std::string mtxPath{"/home/andreys/mtxs/"};
std::atomic<bool> mtx_is_loaded;
std::atomic<unsigned> mtx_count(0);

bool loadMatrix(queuemt<fs::path>& MtxQueueFiles, queuemt<MtxSolver>& MtxQueueToSolve)
{
	std::string MtxFileName;
	fs::path path;
	size_t count = 0;

	std::cout << std::this_thread::get_id() << ". Start LOAD thread.\n";
	while (!MtxQueueFiles.empty()) 
	{
		if (MtxQueueFiles.try_pop(path)) 
		{
			MtxSolver Mtx;
			std::cout << std::this_thread::get_id() 
				<< ". Start load matrix #" << ++count << "  -> " << path.filename().string() << " <-------------------------\n";
			auto beginTime = std::chrono::steady_clock::now();
			Mtx.LoadFromFile(path.string());
			auto endTime = std::chrono::steady_clock::now();
			auto workTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime);
			std::cout << "Load time \""
				<< path.filename().string()
				<< "\" - "
				<< std::fixed << std::setprecision(2)
				<< workTime.count()
				<< " milliseconds.\n";
			std::cout << std::this_thread::get_id() << ". Start " << path.filename().string() << " put in the queuemt \n";
			while (!MtxQueueToSolve.try_push(std::move(Mtx)))
			{
				// std::cout << std::this_thread::get_id() 
				// 	<< ". Queue to solve is full, wait 0.1 sec and retry...\n";
				std::this_thread::sleep_for(std::chrono::milliseconds(25));
			}
			std::cout << std::this_thread::get_id() << ". End put " << path.filename().string() << " in the queuemt \n";
		}
	}
	// bool expected = false;
	// if (mtx_is_loaded.compare_exchange_strong(expected, true)) 
	// {
	// 	std::cout << std::this_thread::get_id() << ". Set end of load flag. \n";
	// }
	std::cout << std::this_thread::get_id() << ". Load " << count << " matrices.\n";
	return true;
}

bool calcMatrix(queuemt<MtxSolver>& MtxQueueToSolve, queuemt<MtxSolver>& MtxQueueSolved) 
{
	MtxSolver temp_mtx;
	std::cout << std::this_thread::get_id() << ". Start CALC thread.\n";
	while (!MtxQueueToSolve.empty() || !mtx_is_loaded.load())  
	{
		if (MtxQueueToSolve.try_pop(temp_mtx))
		{
			std::cout << std::this_thread::get_id() << ". Start calc " << temp_mtx << " <++++++++++++++++++++\n";
			auto beginTime = std::chrono::steady_clock::now();
			temp_mtx.Solve();
			// temp_mtx.free();
			auto endTime = std::chrono::steady_clock::now();
			auto workTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime);
			std::cout << "Calc time \""
				<< temp_mtx
				<< "\" - "
				<< std::fixed << std::setprecision(2)
				<< workTime.count()
				<< " milliseconds.\n";
			std::cout << std::this_thread::get_id() << ". End calc " << temp_mtx << "\n";
			MtxQueueSolved.try_push(std::move(temp_mtx));
		} 
		else 
		{
			// std::cout << std::this_thread::get_id() 
			// 		<< ". Wait for new loaded matrix \n";
			std::this_thread::sleep_for(std::chrono::milliseconds(25));
		}
	}
	std::cout << std::this_thread::get_id() << ". End CALC thread.\n";
	return true;
}

void test1()
{

	std::cout << "Test1 begin. \n";
	// MtxSolver M;
	// M.LoadFromFile("/home/andreys/mtxs/matrix-3.5238bc.mtx");
	// std::cout << "Load  ok. \n";
	// M.Solve();
	std::vector<MtxLine> v;
	v.push_back(MtxLine(2000));
	for (size_t i = 0; i < 2000; i++)
	{
		v[0][i] = 10.15*i;
	}
	
	for (size_t i = 0; i < 2000; i++)
	{
		std::cout << v[0][i] << "\n";
	}
	
	std::cout << "Test1 end. \n";

}

void test2()
{

	std::cout << "Test2 begin. \n";
	MtxSolver M;
	M.LoadFromFile("/home/andreys/mtxs/matrix-1000.72c916.mtx");
	std::cout << "Load  ok. \n";
	auto beginTime = std::chrono::steady_clock::now();
	M.Solve();
	auto endTime = std::chrono::steady_clock::now();
	auto workTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime);
	std::cout << "Load + Calc time - "
	<< std::fixed << std::setprecision(2)
		<< workTime.count()
		<< " milliseconds.\n";

	M.saveAnswers("./answers/test2-orig.txt");
	std::cout << "Test2 end. \n";

}

int main(int argc, char *argv[])
{

	
	#ifdef EXTRAOUT
		std::cout << "Debug mode is ON \n";
	#endif
	test2();
/*
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
	auto beginTime = std::chrono::steady_clock::now();
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
	for (auto &calcThread : calcThreads)
	{
		calcThread.join();
	}
	
	auto endTime = std::chrono::steady_clock::now();
	auto workTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime);
	std::cout << "Load + Calc time - " 
	<< std::fixed << std::setprecision(2)
		<< workTime.count()
		<< " milliseconds.\n";

	std::cout << "To solve queue size -" << mtx_queue_to_solve.size() <<  "\n";
	std::cout << "Solved matrices size -" << mtx_queue_solved.size() <<  "\n";
*/
	std::cout << "Main end. \n";
	std::cin.get();

	return 0;
}