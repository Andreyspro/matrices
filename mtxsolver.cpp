#undef EXTRAOUT

#include <fstream>
#include <iostream>
#include <sstream>

#include "boost/filesystem.hpp"

#include "mtxsolver.h"

namespace fs = boost::filesystem;

MtxSolver::MtxSolver() // default constructor
	: isSolved(false), size(0), data_header(supported_mtx_data_header)
{
	#ifdef EXTRAOUT
		std::cout << "MtxSolver default constructor ======\n";
	#endif
}

MtxSolver::MtxSolver(const MtxSolver &copy) // copy constructor
	: isSolved(copy.isSolved), size(copy.size)
{
	#ifdef EXTRAOUT
		std::cout << "MtxSolver start copy constructor ====== \n";
	#endif
	MtxFileName = copy.MtxFileName;
	Mtx = copy.Mtx;
	Answers = copy.Answers;
	#ifdef EXTRAOUT
		std::cout << "MtxSolver end copy constructor.\n";
	#endif
}

MtxSolver::MtxSolver(MtxSolver &&right) // move constructor
	: isSolved(right.isSolved), size(right.size)
{
	#ifdef EXTRAOUT
		std::cout << "MtxSolver start move constructor ======\n";
	#endif
	MtxFileName = std::move(right.MtxFileName);
	Mtx = std::move(right.Mtx);
	Answers = std::move(right.Answers);
	#ifdef EXTRAOUT
		std::cout << "MtxSolver end move constructor.\n";
	#endif
}

MtxSolver::~MtxSolver()
{
	#ifdef EXTRAOUT
		std::cout << "MtxSolver destructor.\n";
	#endif
}

const MtxSolver &MtxSolver::operator=(const MtxSolver &copy) //copy assign
{
	#ifdef EXTRAOUT
		std::cout << "MtxSolver start copy assign ======.\n";
	#endif
	if (&copy != this)
	{
		isSolved = copy.isSolved;
		size = copy.size;
		MtxFileName = copy.MtxFileName;
		Mtx = copy.Mtx;
		Answers = copy.Answers;
	}
	else
		std::cout << "Attempt self assing !\n";
	return *this;
	#ifdef EXTRAOUT
		std::cout << "MtxSolver end copy assign.\n";
	#endif
}

const MtxSolver &MtxSolver::operator=(MtxSolver &&right)
{
	#ifdef EXTRAOUT
		std::cout << "MtxSolver start move assign ======.\n";
	#endif
	if (&right != this)
	{
		isSolved = right.isSolved;
		size = right.size;
		MtxFileName = std::move(right.MtxFileName);
		Mtx = std::move(right.Mtx);
		Answers = std::move(right.Answers);
	}
	else
		std::cout << "MtxSolver. Attempt self assing !\n";
	return *this;
	#ifdef EXTRAOUT
		std::cout << "MtxSolver end move assign.\n";
	#endif
}

void MtxSolver::LoadFromFile(const std::string &FileName)
{
	std::ifstream mtxFile(FileName.c_str(), std::ios::in);
	if (!mtxFile)
	{
		throw std::runtime_error("MtxSolver. Cant open file '" + FileName + "'");
	}
	LoadFromStream(mtxFile);
	MtxFileName = FileName;
}

void MtxSolver::LoadFromStream(std::istream &imtxstream)
{
	std::string cur_type, cur_version, cur_subversion;

	std::getline(imtxstream, cur_type);
	if (cur_type != supported_mtx_data_header.type)
		throw std::runtime_error("Current file type not supported. Incorrect type");
	
	std::getline(imtxstream, cur_version);
	if (supported_mtx_data_header.version != std::stoi(cur_version))
		throw std::runtime_error("Current file type not supported. Incorrect version");
	
	std::getline(imtxstream, cur_subversion);
	if (supported_mtx_data_header.subversion != std::stoi(cur_subversion))
		throw std::runtime_error("Current file type not supported. Incorrect subversion");
	
	cur_type.copy(data_header.type, sizeof(data_header.type));
	data_header.version = std::stoi(cur_version);
	data_header.subversion = std::stoi(cur_subversion);

	imtxstream >> size;
	Mtx.reserve(size);
	for (size_t i = 0; i < size; i++)
	{
		Mtx.emplace_back(size + 1);
		for (size_t j = 0; j <= size; j++)
		{
			imtxstream >> Mtx[i][j];
		}
	}
}

void MtxSolver::SaveToFile(const std::string &FileName) const
{
	std::ofstream mtxFile(FileName.c_str(), std::ios::out);
	if (!mtxFile)
	{
		throw std::runtime_error("MtxSolver. Cant open file '" + FileName + "' for save");
	}
	SaveToStream(mtxFile);
}

void MtxSolver::SaveToStream(std::ostream &omtxstream) const
{
	omtxstream << data_header.type <<"\n";
	omtxstream << data_header.version << "\n";
	omtxstream << data_header.subversion << "\n";
	omtxstream << size << "\n";
	omtxstream << std::fixed;
	if (size > 0)
	{ 
		for (size_t i = 0; i < size; i++)
		{
			for (size_t j = 0; j < size + 1; j++)
			{
				omtxstream << Mtx[i][j] << "\n";
			}
		}
	}
}	

size_t MtxSolver::getSize() const
{
	return size;
}

MtxElement MtxSolver::getAnswers(size_t index) const
{
	if (!isSolved)
		throw std::runtime_error("MtxSolver. Matrix not solved.");

	if (index > Answers.size())
	{
		std::stringstream ErrorString;
		ErrorString << "MtxSolver. Index error " << index << ".";
		throw std::runtime_error(ErrorString.str());
	}
	return Answers[index];
}

void MtxSolver::Solve()
{
	MtxElement divisor;
	MtxElement multiplier;

	for (size_t i = 0; i < size; i++)
	{

		divisor = Mtx[i][i];
		// divisor = Mtx[i].Data[i];
		if (divisor != 1)
			for (size_t j = i; j < size + 1; j++)
				Mtx[i][j] /= divisor;
				// Mtx[i].Data[j] /= divisor;

		for (size_t i2 = 0; i2 < size; i2++)
			if (i2 != i)
			{
				multiplier = Mtx[i2][i];
				// multiplier = Mtx[i2].Data[i];
				for (size_t j2 = 0; j2 < size + 1; j2++)
					// Mtx[i2].Data[j2] -= Mtx[i].Data[j2] * multiplier;
					Mtx[i2][j2] -= Mtx[i][j2] * multiplier;
					// Mtx[i2][j2] = Mtx[i2][j2] - Mtx[i][j2] * multiplier;

			}
	}
	Answers.resize(size);
	for (size_t i = 0; i < size; i++)
	{
		Answers[i] = Mtx[i][size];
	}
	isSolved = true;
}

void MtxSolver::saveAnswers(const std::string &AnswersFileNameStr)
{
	if (!isSolved || size == 0)
		throw std::runtime_error("MtxSolver. Error: Matrix not solved!");
	fs::path MtxFilenamePath(AnswersFileNameStr);
	fs::create_directories(MtxFilenamePath.parent_path());
	std::ofstream AnswersFile(MtxFilenamePath.string().c_str(), std::ios::out);
	if (!AnswersFile)
	{
		throw std::runtime_error("MtxSolver. Error: Can't write to file: " + AnswersFileNameStr);
	}
	else
	{
		for (size_t i = 0; i < size; i++)
			AnswersFile << Answers[i] << "\n";
		AnswersFile.close();
	}
}

void MtxSolver::free()
{
	#ifdef EXTRAOUT
		std::cout << "MtxSolver free. '" << MtxFileName << "'\n";
	#endif
	Mtx.clear();
	Mtx.shrink_to_fit();
}

std::ostream &operator<<(std::ostream &output, const MtxSolver &Mtx)
{
	output << fs::path(Mtx.MtxFileName).filename();
	return output;
}
