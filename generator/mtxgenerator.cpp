
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <limits>
#include <fstream>
#include <sstream>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/filesystem.hpp>
#include "mtxaux.h"

namespace fs = boost::filesystem;

int main(const int argc, char *argv[]){ 
	int elementCount;
	double val;
	int multiple;
	int mtxCount;
	boost::uuids::random_generator gen;
	std::string mtxFileNameUID;
	// std::stringstream mtxFileName;
	std::string mtxFileName;
	// std::string mtxDir("/home/andreys/mtxs");
	if (argc < 2)
	{
		std::cout << "Error. No argument - mtx directory .";
	}
	std::string mtxDir = argv[1];

	fs::path mtxFilePath;

	srand(time(0));
	
	int size = 2;
	size_t low_bound = 2;
	size_t hi_bound = 2;

	std::cout << "\nEnter matrix size (0 if random size)\n";
	std::cout << "Size:"; 
	std::cin >> size;
	std::cout << std::endl;

	bool is_random_size = (size == 0);
	if (is_random_size) {
		std::cout << "\nEnter lower bound of random matrix size\n";
		std::cout << "Low bound:"; 
		std::cin >> low_bound;
		std::cout << std::endl;

		std::cout << "\nEnter hi bound of random matrix size\n";
		std::cout << "Hi bound:"; 
		std::cin >> hi_bound;
		std::cout << std::endl;
	}

	std::cout << "\nHow many matrices\n";
	std::cout << "Count:"; 
	std::cin >> mtxCount;
	std::cout << std::endl;

	for (int m = 0; m < mtxCount; m++) {
		if (is_random_size)
			size = low_bound + (rand() % (hi_bound - low_bound));
		mtxFileNameUID = boost::uuids::to_string(gen()).substr(0,6);
		mtxFileName = "matrix-" + std::to_string(size) + "." + mtxFileNameUID + ".mtx";
		mtxFilePath = mtxDir;
		mtxFilePath /= mtxFileName;
		std::cout << "Try create file '" << mtxFilePath.string() << "'\n";
		std::ofstream mtxFile(mtxFilePath.c_str(), std::ios::out);
		if (!mtxFile) {
			std::cout << "Error. File not opened!\n";
			return 1;
		}

		elementCount = size * (size +1);
		std::cout << std::endl;
		std::cout << "Generate " << mtxFileName << std::endl;
 
		mtxFile << std::fixed;
		// mtxFile << mtx_file_header.name << "\n" << mtx_file_header.version << "\n" 
		// 	<< mtx_file_header.subversion << "\n";
		mtxFile << mtx::supported_data_header.type << "\n" << mtx::supported_data_header.version << "\n" 
			<< mtx::supported_data_header.subversion << "\n";
		mtxFile << size << "\n";
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				multiple = (rand() % 2 > 0) ? 1 : -1;
				val = (rand() + 1 / static_cast<double> (rand() % 1000 + 1) ) * multiple;
				mtxFile << val << "\n";
			}
			multiple = (rand() % 2 > 0) ? 1 : -1;
			val = (rand() * (rand() % 1000) + 1 / static_cast<double> (rand() % 1000 + 1)) * multiple;
			mtxFile << val << "\n";
			if (i % 100 == 0)
				std::cout << "Line " << i << " from " << size << " done." << std::endl;
		}
		mtxFile.close();
	}
	std::cout << "Done\n\n";

	return 0;

}