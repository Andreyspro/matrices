#include <fstream>
#include <iostream>
#include <sstream>

#include "boost/filesystem.hpp"

#include "mtxsolver.h"

namespace fs = boost::filesystem;

MtxSolver::MtxSolver() // default constructor
	: isSolved(false), size(0)
{
	std::cout << "MtxSolver default constructor ======\n";
}

MtxSolver::MtxSolver(const MtxSolver &copy) // copy constructor
	: isSolved(copy.isSolved), size(copy.size)
{
	std::cout << "MtxSolver start copy constructor ======n";
	MtxFileName = copy.MtxFileName;
	Mtx = copy.Mtx;
	Answers = copy.Answers;
	std::cout << "MtxSolver end copy constructor.\n";
}

MtxSolver::MtxSolver(MtxSolver &&right) // move constructor
	: isSolved(right.isSolved), size(right.size)
{
	std::cout << "MtxSolver start move constructor ======\n";
	MtxFileName = std::move(right.MtxFileName);
	Mtx = std::move(right.Mtx);
	Answers = std::move(right.Answers);
	std::cout << "MtxSolver end move constructor.\n";
}

const MtxSolver &MtxSolver::operator=(const MtxSolver &copy) //copy assign
{
	std::cout << "MtxSolver start copy assign ======.\n";
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
	std::cout << "MtxSolver end copy assign.\n";
}

const MtxSolver &MtxSolver::operator=(MtxSolver &&right)
{
	std::cout << "MtxSolver start movw assign ======.\n";
	if (&right != this)
	{
		isSolved = right.isSolved;
		size = right.size;
		MtxFileName = std::move(right.MtxFileName);
		Mtx = std::move(right.Mtx);
		Answers = std::move(right.Answers);
	}
	else
		std::cout << "Attempt self assing !\n";
	return *this;
	std::cout << "MtxSolver end move assign.\n";
}

void MtxSolver::LoadFromFile(const std::string &FileName)
{
	std::ifstream mtxFile(FileName.c_str(), std::ios::in);
	if (!mtxFile)
	{
		throw std::runtime_error("Cant open file '" + FileName + "'");
	}
	Mtx.clear();
	mtxFile >> size;
	Mtx.resize(size);

	for (size_t i = 0; i < size; i++)
	{
		Mtx[i].resize(size + 1);
		for (size_t j = 0; j <= size; j++)
		{
			mtxFile >> Mtx[i][j];
		}
	}
	MtxFileName = FileName;
}

size_t MtxSolver::getSize() const
{
	return size;
}

MtxElement MtxSolver::getAnswers(size_t index) const
{
	if (!isSolved)
		throw std::runtime_error("Matrix not solved.");

	if (index > Answers.size())
	{
		std::stringstream ErrorString;
		ErrorString << "Index error " << index << ".";
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
		if (divisor != 1)
			for (size_t j = i; j < size + 1; j++)
				Mtx[i][j] /= divisor;

		for (size_t i2 = 0; i2 < size; i2++)
			if (i2 != i)
			{
				multiplier = Mtx[i2][i];
				for (size_t j2 = 0; j2 < size + 1; j2++)
					Mtx[i2][j2] -= Mtx[i][j2] * multiplier;
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
		throw std::runtime_error("Error: Matrix not solved!");
	fs::path MtxFilenamePath(AnswersFileNameStr);
	fs::create_directories(MtxFilenamePath.parent_path());
	std::ofstream AnswersFile(MtxFilenamePath.string().c_str(), std::ios::out);
	if (!AnswersFile)
	{
		throw std::runtime_error("Error: Can't write to file: " + AnswersFileNameStr);
	}
	else
	{
		for (size_t i = 0; i < size; i++)
			AnswersFile << Answers[i] << "\n";
		AnswersFile.close();
	}
}
