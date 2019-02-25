cmake_minimum_required(VERSION 2.6)

PROJECT (rpc_client)

set (CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++11 -g -Wl,-rpath=./ -Wl,--dynamic-linker,./ld-linux-x86-64.so.2 -Wall -fPIC -Wconversion -fvisibility=hidden -L../../../lib")

SET (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../../lib)
SET (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)

aux_source_directory(./src LIBRPCCLI_SRCS)

include_directories(../rpc_message/src ../common)

add_library (rpc_client SHARED ${LIBRPCCLI_SRCS})

target_link_libraries(rpc_client rpc_message protobuf-lite)

