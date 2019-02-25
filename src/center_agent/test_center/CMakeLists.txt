cmake_minimum_required(VERSION 2.6)

#project (test_center_agent) 

#Force to use CXX compiler to compile c source code.
set (CMAKE_C_COMPILER ${CMAKE_CXX_COMPILER})

#TODO: remove '-g', use '-DCMAKE_BUILD_TYPE=DEBUG' instead
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -Wall -Wl,--dynamic-linker,../../../../lib/ld-linux-x86-64.so.2 -Wl,-rpath,../../../../lib")

include_directories(SYSTEM "../../common" "../../../src_refactoring/upload_factory/inc" "../kv_message/src"  "../center_agent/src" "./" "../center_agent/include")

set (GENEUUID_SRCS ./generate_uuids.cpp)

add_executable(gene_uuid ${GENEUUID_SRCS})
target_link_libraries(gene_uuid ${PROJECT_SOURCE_DIR}/../../../lib/libuuid.a)

set (COMMON_SRCS )

#aux_source_directory(./test.cpp SRCS)

set (SRCS ./test.cpp ./make_log.c)

set(SRCS ${SRCS} ${COMMON_SRCS})

add_executable(test_center_agent ${SRCS})

target_link_libraries(test_center_agent ${PROJECT_SOURCE_DIR}/../../../lib/libcenter_agent.so)
target_link_libraries(test_center_agent ${PROJECT_SOURCE_DIR}/../../../lib/libmodule_data.so)
target_link_libraries(test_center_agent ${PROJECT_SOURCE_DIR}/../../../lib/libkv_message.a)
target_link_libraries(test_center_agent ${PROJECT_SOURCE_DIR}/../../../lib/libuploader_factory.so)
target_link_libraries(test_center_agent pthread)


