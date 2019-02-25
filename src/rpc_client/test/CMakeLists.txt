cmake_minimum_required(VERSION 2.6)
project(test_ipc_client)

set (CMAKE_C_COMPILER ${CMAKE_CXX_COMPILER})

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wl,--dynamic-linker=../../../../lib/ld-linux-x86-64.so.2  -g -Wall -Wconversion -fvisibility=hidden")

include_directories(../src ../../common/include ../../rpc_message/src ../../common)

aux_source_directory(./ SRCS)

link_directories(${PROJECT_SOURCE_DIR}/../../../lib)
add_executable(test_rpc_client ${SRCS})

target_link_libraries(test_rpc_client rpc_client pthread)

