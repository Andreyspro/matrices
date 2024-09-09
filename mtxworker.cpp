#include <thread>
#include <iostream>
#include <atomic>
#include <ctime>

#include "boost/filesystem.hpp"
#include "mtxsolver.h"
#include "mtxworker.h"
#include "perf_timer.h"

extern size_t verbosity;
std::atomic_size_t number_of_solved(0);

bool load_mtxs(queuemt<mtx_source_t>& mtx_src_for_load_list, queuemt<MtxSolver>& mtx_for_solve_list)
{
	size_t count = 0;
	mtx_source_t mtx_source;

	std::cout << std::this_thread::get_id() << ". Start LOAD thread.\n";
	while (mtx_src_for_load_list.wait_and_pop(mtx_source)) 
	{
		MtxSolver Mtx;
		perf_timer<std::chrono::milliseconds> load_timer1;
		try {
			switch (mtx_source.type)
			{
			case NOTHING:
				if (verbosity >= 2)
					std::cout << std::this_thread::get_id() 
						<< ". Loading. "  << mtx_source.name << " source is EMPTY.\n";
				break;

			case MTX_FILE:
				if (verbosity >= 2)
					std::cout << std::this_thread::get_id() 
						<< ". Loading. Matrix #" << count << " - " << mtx_source.name 
						<< ". Start load from file \"" << mtx_source.file_name << "\"\n";
				++count;
				Mtx.LoadFromFile(mtx_source.file_name);
				break;

			case MTX_FILE_STREAM:
				if (verbosity >= 2)
					std::cout << std::this_thread::get_id() 
						<< ". Loading. Matrix #" << count << " - " << mtx_source.name 
						<< " Start load from file stream.\n";
				++count;
				Mtx.LoadFromFileStream(*mtx_source.file_stream, mtx_source.name);
				break;

			case MTX_NET:
				break;

			default:
				break;
			}
			
		} catch (std::runtime_error& ex) {
			std::cout << "Error - " << ex.what() << "\n";
			exit(1);
		}

		load_timer1.stop();
		if (verbosity >= 2) {
			std::cout << std::this_thread::get_id() << ". End load. Load time \""
				<< mtx_source.name<< "\" - "
				<< load_timer1 << " milliseconds.\n";
		}

		if (!mtx_for_solve_list.wait_and_push(std::move(Mtx))) {
			std::cout << std::this_thread::get_id() << ". Error while put " << mtx_source.name << " in the queuemt. \n";
		}
		if (verbosity >= 1)
			std::cout << "Loader. Preload MTX size - " << mtx_for_solve_list.size() << "\n";
		if (verbosity >= 2)
			std::cout << std::this_thread::get_id() << ". End put " << mtx_source.name << " in the queuemt. \n";
	}
	std::cout << std::this_thread::get_id() << ". Load " << count << " matrices.\n";
	return true;
}

bool solve_mtx(queuemt<MtxSolver>& mtx_for_solve_list, queuemt<MtxSolver>& mtx_solved_list) 
{
	size_t counter = 0;
	MtxSolver temp_mtx;
	std::cout << std::this_thread::get_id() << ". Start CALC thread.\n";
	while (mtx_for_solve_list.wait_and_pop(temp_mtx))
	{
		if (verbosity >= 1)
			std::cout << "Calc. Preload MTX size - " << mtx_for_solve_list.size() << "\n";

		if (verbosity >= 2)
		{
			std::cout << std::this_thread::get_id() << ". Start calc #" << counter
				<< " \"" << temp_mtx << "\"...\n";
		}
		perf_timer<std::chrono::milliseconds> calc_timer1;
		temp_mtx.Solve();
		calc_timer1.stop();
		temp_mtx.free();
		if (verbosity >= 2) 
		{
			std::cout << std::this_thread::get_id() << ". End calc #" << counter 
				<< ". Calc time \""	<< temp_mtx << "\" - "
				<< calc_timer1 << " milliseconds.\n";
		}
		mtx_solved_list.try_push(std::move(temp_mtx));
		++counter;
		++number_of_solved;
	}
	std::cout << std::this_thread::get_id() << ". End CALC thread. Number of solved matrices - " 
		<< counter << "\n";
	return true;
}

void save_answers(queuemt<MtxSolver>& mtx_to_save_list, std::string save_dir)
{
	MtxSolver temp_mtx;
	std::cout << std::this_thread::get_id() << ". Start Save thread.\n";
	while (mtx_to_save_list.wait_and_pop(temp_mtx))
	{

		if (verbosity >= 2)
			std::cout << std::this_thread::get_id() << ". Start save " << temp_mtx << "...\n";
		perf_timer<std::chrono::milliseconds> save_timer1;
		temp_mtx.SaveAnswers(save_dir + temp_mtx.GetMtxName()+ ".ans");
		save_timer1.stop();
		if (verbosity >= 2) 
		{
			std::cout << std::this_thread::get_id() << ". End save. Save time \""	<< temp_mtx << "\" - "
				<< save_timer1 << " milliseconds.\n";
		}
	}
	std::cout << std::this_thread::get_id() << ". End Save thread.\n";
}

void print_status ()
{
	char timestr[16];
	std::time_t t; 
	while (true) 
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
		t = std::time(nullptr);
		std::strftime(timestr, sizeof(timestr), "%H:%M:%S", std::localtime(&t));
		std::cout << timestr <<". Solved " << number_of_solved.load() << "\n";
	}
}
