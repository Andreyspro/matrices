
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "boost/filesystem.hpp"
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "mtxsolver.h"
#include "queuemt.h"
#include <atomic>

namespace fs = boost::filesystem;
const unsigned int MAX_THREAD_COUNT = 6;
const unsigned int MAX_LOAD_THREADS = 2;
std::string mtxPath{"/home/andreys/mtxs/"};
std::atomic<unsigned> MtxCount = 0;

bool loadMatrix(queuemt<fs::path>& mtxFilesQueue, queuemt<MtxSolver>& loadedMtxQueue)
{
	std::string MtxFileName;
	while (mtxFileQueue)
	return true;
}

bool calcMatrix(std::string MtxFile, std::string MtxAnswersDir)
{

	fs::path MtxFilePath(MtxFile); // Path to Matrix
	std::string MtxAnswersFileName{"./answers/" + MtxAnswersDir + "/" + MtxFilePath.stem().string() + ".ans"};
	try
	{
		std::cout << MtxFilePath.filename() << "\n";
		MtxSolver Mtx;
		Mtx.LoadFromFile(MtxFilePath.string());
		Mtx.Solve();
		Mtx.saveAnswers(MtxAnswersFileName);
		std::cout << MtxFilePath.stem().string() << " done.\n";
	}
	catch (std::runtime_error &error)
	{
		std::cout << error.what() << std::endl;
	}
	return true;
}

int main(int argc, char *argv[])
{
	std::cout << "Main begin. \n";
	// RUN UID
	boost::uuids::random_generator gen;
	std::string runUID = boost::uuids::to_string(gen()).substr(0, 6);

	// Fill list of matrices files.
	queuemt<fs::path> mtxFilesList;
	for (auto const &dirPath : fs::directory_iterator(mtxPath))
	{
		if (!fs::is_directory(dirPath)) {
			mtxFilesList.try_push(dirPath.path());
			++MtxCount;
		}
	}

	// Load Mtx form files one by one with litited size
	queuemt<MtxSolver> LoadingMtx(2);
	loadmatrix(mtxFilesList, LoadingMtx);


	fs::path p;
	while (mtxFilesList.try_pop(p))
	{
		std::cout << p << "\n";
	}
	std::cout << "Main end. \n";
	// //array of Threads
	// std::vector<std::thread> MtxThreads;
	// //Path to diretory with matrices
	// std::string mtxPath{"/home/andreys/mtxs/"};
	// std::vector<fs::path> MtxsFiles;  //List of files of matrices
	// std::vector<MtxSolver> Mtxs;

	// std::cout << "Run UID is """ << runUID << """\n";

	// //load file name to
	// fs::directory_iterator end_it;
	// for (fs::directory_iterator dir_it(mtxPath); dir_it != end_it; ++dir_it) {
	// 	if (!fs::is_directory(dir_it->path()))
	// 		MtxsFiles.push_back(dir_it->path());
	// }

	// auto beginTime = std::chrono::steady_clock::now();

	// for (auto& MtxFile : MtxsFiles) {

	// 	MtxThreads.push_back(std::thread(calcMatrix,MtxFile.string(), runUID));
	// }

	// for (auto &MtxThread : MtxThreads) {
	// 	MtxThread.join();
	// }

	// auto endTime = std::chrono::steady_clock::now();

	// auto workTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime);

	// std::cout << runUID << " is done.\n"
	// 	<< "\nTotal solving time - "
	// 	<< std::fixed << std::setprecision(2)
	// 	<< workTime.count()
	// 	<< " milliseconds.\n";

	std::cin.get();

	return 0;
}