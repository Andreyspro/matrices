#ifndef MTXAUX_H
#define MTXAUX_H

#include "boost/asio.hpp"

#define MTX_VER_MAJOR 1
#define MTX_VER_MINOR 0
#define MTX_VER_PATCH 0



#define MTX_HELLO_SIZE 32
#define MTX_HELLO_PREFIX "         MTXSOLVER-HELLO"
#define MTX_HEADER_HELLO "         MTXSOLVER-HELLO,1.0.0##" //HELLO string and version + end symbol ##
// commands
#define MTX_CMD_SIZE 32
#define MTX_CMD_RECEIVE_MTX_AND_CALC "     MTXCMD_RECEIVE_MTX_AND_CALC"
// #define MTXCMD_START_SESSION = "MTXCMD_c28fc4d7-d369-4c3b-b07f-a5a464194582"
// #define MTXCMD_RECEIVE_MTX = "MTXCMD_77866dd2-cba9-4e3a-ab9b-8dd1cf50a882"
// #define MTXCMD_RECEIVE_ANSWER = "MTXCMD_6bdb06cd-0535-4df9-ae61-682ee463b1e6"
// #define MTXCMD_ERROR = "MTXCMD_bdd45797-edea-4833-a314-b1c8f1183c21"
// #define MTXCMD_END_SESSION = "MTXCMD_32b5489a-26a2-48de-8cd2-863ae6a334f2"

#define MTXFLOW_END_FlAG = "\r\n\r\n"

namespace mtx {

typedef size_t version_t;

const version_t curr_supported_ver = 1000000;

version_t parse_hello (std::string s_hello);
version_t to_wide_ver(size_t ver, size_t sub_ver);
bool data_is_supported(size_t ver, size_t sub_ver);
bool data_is_supported(std::string ver, std::string sub_ver);

struct data_header_t
{
	char type[17];
	int version;
	int subversion;
};

const data_header_t supported_data_header = {"MatrixSolver    ", 1, 0};
const boost::asio::ip::port_type def_ip_port = 18404;

} //namespace mtx

#endif //#ifdef MTXAUX_H