#ifndef MTXWORKER_H
#define MTXWORKER_H

#include <string>

#include "boost/filesystem.hpp"
#include "queuemt.h"
#include "mtxsolver.h"

enum mtx_source_type {NOTHING, MTX_FILE, MTX_FILE_STREAM, MTX_NET};

typedef std::unique_ptr<std::iostream> file_stream_ptr_t;

struct mtx_source_t
{
	std::string name = "";
	mtx_source_type type = NOTHING;
	std::string file_name = "";
	file_stream_ptr_t file_stream = NULL;
	//std::unique_ptr<std::iostream> socket_stream;
};

bool load_mtxs(queuemt<mtx_source_t>& mtx_src_for_load_list, queuemt<MtxSolver>& mtx_for_solve_list);
bool solve_mtx(queuemt<MtxSolver>& , queuemt<MtxSolver>& );
void save_answers(queuemt<MtxSolver>& , std::string );
void print_status();

#endif