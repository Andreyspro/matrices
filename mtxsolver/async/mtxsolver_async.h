#ifndef MTXSOLVER_ASYNC
#define MTXSOLVER_ASYNC

#include "boost/asio.hpp"
#include <memory>
#include <string> // experemental

#include "and_net3.h"
#include "mtxsolver.h"

using namespace boost::asio;

class mtxsolver_async_loader : 
    public std::enable_shared_from_this<mtxsolver_async_loader>,
	public MtxSolver
{

	//default constructor is deleted
    mtxsolver_async_loader() = delete; 
    // bas constructor is hidden
	mtxsolver_async_loader(std::shared_ptr<and_net::net_three> net_ptr);

public:

    typedef std::shared_ptr<mtxsolver_async_loader> self_ptr_t;
	// type of callback that invoke after load mtx
	// 1 arg - succeful/unsuccessful
	// 2 arg - size of matrix
	typedef std::function<void(bool, size_t, self_ptr_t)> callback_t;

    static self_ptr_t get_new(std::shared_ptr<and_net::net_three> &net);
    void start(callback_t);
private:
    void do_read_mtx();
    void on_read_mtx(and_net::op_fill_status_t);
	std::shared_ptr<and_net::net_three> m_net_ptr;
    callback_t m_callback;
	enum stage_t
	{
		LOAD_TYPE = 0,
        LOAD_VERSION,
        LOAD_SUBVERSION,
		LOAD_SIZE,
		LOAD_MTX_ELEMENTS,
		LOAD_DONE
	} m_stage;
	struct load_index_t
	{
		size_t i;
		size_t j;
	} m_load_index;
	size_t m_invokes = 0; // experemental data
	std::string m_element; // experemental data
};

#endif // #ifndef MTXSOLVER_ASYNC 