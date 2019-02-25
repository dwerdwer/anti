cmake_minimum_required(VERSION 2.6)

project(kv_message)

set (CMAKE_C_COMPILER ${CMAKE_CXX_COMPILER})
	
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -fPIC -fvisibility=hidden -Wall")

aux_source_directory(./src LIBKVMSG_SRCS)

include_directories(SYSTEM "../../common")
#include_directories(SYSTEM "${PROJECT_SOURCE_DIR}/include")

link_directories( ${PROJECT_SOURCE_DIR}/../../../lib)
add_library (kv_message STATIC ${LIBKVMSG_SRCS})
target_link_libraries(kv_message protobuf-lite)

#target_link_libraries(kv_message ${PROJECT_SOURCE_DIR}/lib/libprotobuf-lite.a)

add_custom_command(TARGET kv_message
	POST_BUILD
	COMMAND cp -vf $<TARGET_FILE:kv_message> ${PROJECT_SOURCE_DIR}/../../../lib)
