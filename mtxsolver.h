#ifndef MTXSOLVER_H
#define MTXSOLVER_H

#include <vector>
#include <string>

struct mtx_data_header_t
{
	char type[17];
	int version;
	int subversion;
};

const mtx_data_header_t supported_mtx_data_header = {"MatrixSolver    ", 1, 0};

typedef double MtxElement;
typedef std::vector<MtxElement> MtxLine;

class MtxSolver {
	friend std::ostream &operator<<(std::ostream &, const MtxSolver &);
public:
	MtxSolver();
	MtxSolver(const MtxSolver& right); // move constructor 
	MtxSolver(MtxSolver&& right); // move constructor 
	~MtxSolver();
	const MtxSolver& operator=(const MtxSolver&);
	const MtxSolver& operator=(MtxSolver&&);
	void LoadFromFile(const std::string &FileName);
	void LoadFromFileStream(std::istream &imtxstream, const std::string name = "");
	void SaveToFile(const std::string &FileName) const;
	void SaveToStream(std::ostream &omtxstream) const;
	size_t getSize() const;
	std::string GetMtxName() const;
	MtxElement getAnswers(size_t index) const;
	void Solve();
	void SaveAnswers(const std::string& AnswersFileName);
	void free();
private:
	bool isSolved;
	size_t size;
	std::vector<MtxLine> Mtx;
	std::vector<MtxElement> Answers;
	std::string m_mtx_name;
	mtx_data_header_t data_header;
};

#endif