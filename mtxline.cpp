#define EXTRAOUT
#undef EXTRAOUT

#include <algorithm>
#include <iostream>
#include "mtxline.h"

MtxLine::MtxLine()
	: Data(nullptr),
	  DataSize(0)
{
}

MtxLine::MtxLine(size_t NewSize)
{
	#ifdef EXTRAOUT
		std::cout << "Def. constuctor. Size -" << NewSize << "\n";
	#endif
	if (NewSize > 0 )
	{
		Data = new MtxElement[NewSize];
		DataSize = NewSize;
	}
	else
	{
		std::cout << "NewSize less than 1 \n";
		exit(1);
	}
}

MtxLine::MtxLine(const MtxLine &copy) // copy constructor
	: Data (new MtxElement[copy.DataSize]),
	DataSize(copy.DataSize)	  
{
	#ifdef EXTRAOUT
		std::cout << "copy constuctor. \n";
	#endif

	std::copy(copy.Data, copy.Data + copy.DataSize, Data);
}

MtxLine::MtxLine(MtxLine &&right) // move constructor
	: Data(right.Data),
	  DataSize(right.DataSize)	  
{
	#ifdef EXTRAOUT
		std::cout << "Move constuctor. \n";
	#endif
	right.Data = nullptr;
	right.DataSize = 0;
}

MtxLine::~MtxLine() //destructor
{
	#ifdef EXTRAOUT
		std::cout << "Destructor . \n";
	#endif

	delete[] Data;
	DataSize = 0;
}

const MtxLine &MtxLine::operator=(const MtxLine& right) // copy assgnment
{
	#ifdef EXTRAOUT
		std::cout << "copy assgnment. \n";
	#endif

	if (&right != this) {
		delete[] Data;
		Data = new MtxElement[right.DataSize];
		std::copy(right.Data, right.Data + right.DataSize, Data);
		DataSize = right.DataSize;
	} else {
		throw std::runtime_error("MtxLine Self assignent !");
		exit(1);
	}
	return *this;
}

const MtxLine &MtxLine::operator=(MtxLine &&right) // move assignment
{
	#ifdef EXTRAOUT
		std::cout << "move assgnment. \n";
	#endif
	if (&right != this) {
		delete[] Data;
		Data = right.Data;
		DataSize = right.DataSize;
		right.Data = nullptr;
		right.DataSize = 0;
	} else {
		throw std::runtime_error("MtxLine Self assignent !");
		exit(1);
	}
	return *this;
}

MtxElement& MtxLine::operator[](size_t index)
{
	// if (index < 0 || index > DataSize)
	// {
	// 	std::cerr << "setter mtx.Index out of range";
	// 	exit(1);
	// }
	return Data[index];
}

MtxElement MtxLine::operator[](size_t index) const
{
	// if (index < 0 || index > DataSize)
	// {
	// 	std::cerr << "getter mtx.Index out of range";
	// 	exit(1);
	// }
	return Data[index];	
}
