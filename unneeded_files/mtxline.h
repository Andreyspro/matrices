#ifndef MTXLINE_H
#define MTXLINE_H

typedef double MtxElement;

struct MtxLine
{
	MtxLine(); // default constructor
	MtxLine(size_t NewSize); // constructor with sie
	MtxLine(const MtxLine& copy); // copy constructor
	MtxLine(MtxLine&& right); // move constructor
	~MtxLine();
	const MtxLine& operator=(const MtxLine& copy); //copy assgnment
	const MtxLine& operator=(MtxLine&& right);  // move assignment
	MtxElement& operator[](size_t index);
	inline MtxElement operator[](size_t index) const;
	MtxElement* Data;
	size_t DataSize;
};

#endif