cmake_minimum_required(VERSION 2.6)
project(ipc_server_test)
#cmake_policy(VERSION 2.6)
	cmake_policy(SET CMP0015 OLD)

#Force to use CXX compiler to compile C source code.
	set (CMAKE_C_COMPILER ${CMAKE_CXX_COMPILER})

#TODO: remove '-g', use '-DCMAKE_BUILD_TYPE=DEBUG' instead.
#	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS}   -g -Wall -Wconversion -fvisibility=hidden")
#set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wconversion")
#set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wconversion --coverage")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wl,--dynamic-linker=../../../../lib/ld-linux-x86-64.so.2  -g -Wall -Wconversion -fvisibility=hidden")


	include_directories(include ../src ../../common/include ../../common/src ../../common ../../rpc_message/src)

    link_directories(${PROJECT_SOURCE_DIR}/../../../../lib ${PROJECT_SOURCE_DIR}/../../lib ${PROJECT_SOURCE_DIR}/../../../lib)

## Test ipc_server program
set (IPC_SERVER_TEST_SOURCES ./ipc_server_test.cpp)

add_executable(ipc_server_test ${IPC_SERVER_TEST_SOURCES})
target_link_libraries(ipc_server_test  websockets ssl crypto pthread)
target_link_libraries(ipc_server_test ${PROJECT_SOURCE_DIR}/../../../lib/libipc_server.so)

