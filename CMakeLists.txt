cmake_minimum_required(VERSION 3.0)
project(matrices_app)


include_directories(and_net/and_net3)
include_directories(aux)
include_directories(mtxsolver)
include_directories(mtxsolver/async)
include_directories(net_handler)
include_directories(worker)

add_executable(matrices_app
	worker/mtx-worker.cpp

	and_net/and_net3/and_net3.cpp
	and_net/and_net3/and_net3.h

	aux/crack_atod.cpp
	aux/crack_atod.h
	aux/perf_timer.h
	aux/queuemt.h
	aux/uuid.cpp
	aux/uuid.h

	mtxsolver/mtxaux.cpp
	mtxsolver/mtxaux.h
	mtxsolver/mtxsolver.cpp
	mtxsolver/mtxsolver.h

	mtxsolver/async/mtxsolver_async.cpp
	mtxsolver/async/mtxsolver_async.h

	net_handler/mtx_net_handler.cpp
	net_handler/mtx_net_handler.h
)

target_link_libraries(matrices_app boost_filesystem)