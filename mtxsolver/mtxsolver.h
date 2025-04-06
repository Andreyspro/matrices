#ifndef MTXSOLVER_H
#define MTXSOLVER_H

#include <vector>
#include <string>
#include <boost/asio.hpp>

#include "mtxaux.h"
#include "and_net.h"
namespace net = boost::asio;
using tcp = net::ip::tcp;

typedef double MtxElement;
typedef std::vector<MtxElement> MtxLine;

class MtxSolver {
	friend std::ostream &operator<<(std::ostream &, const MtxSolver &);
public:
	MtxSolver();
	MtxSolver(const MtxSolver& right); // copy constructor 
	MtxSolver(MtxSolver&& right); // move constructor 
	~MtxSolver();
	const MtxSolver& operator=(const MtxSolver&);
	const MtxSolver& operator=(MtxSolver&&);
	void LoadFromFile(const std::string &FileName, const std::string &name);
	void LoadFromFile(const std::string &FileName);
	void LoadFromFileStream(std::istream &imtxstream, const std::string name = "");
	void LoadFromNet(and_net::net_one &net_connection, std::string name = "Network");
	void SendToNet(tcp::socket &sock);
	void SaveToFile(const std::string &FileName) const;
	void SaveToStream(std::ostream &omtxstream) const;
	size_t getSize() const;
	std::string GetMtxName() const;
	MtxElement getAnswers(size_t index) const;
	void Solve();
	void SaveAnswers(const std::string& AnswersFileName);
	void free();
protected:
	bool isSolved;
	size_t size;
	std::vector<MtxLine> Mtx;
	std::vector<MtxElement> Answers;
	std::string m_mtx_name;
	mtx::data_header_t data_header;
};

#endif