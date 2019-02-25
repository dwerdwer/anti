cmake_minimum_required(VERSION 2.6)

cmake_policy(SET CMP0015 OLD)
set (CMAKE_C_COMPILER ${CMAKE_CXX_COMPILER})
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -fvisibility=hidden")

include_directories(./src ../center_agent/kv_message/src/ ../common/)
link_directories(${PROJECT_SOURCE_DIR}/../../lib)

#build kvtrace.so
set (TRACE_SRCS src/kvtrace.cpp src/trace_log.cpp)
add_library(kvtrace SHARED ${TRACE_SRCS})
target_link_libraries(kvtrace pthread kv_message protobuf-lite)


#build test program
set (TEST_SRCS test/test_trace.cpp)
add_executable(test_trace ${TEST_SRCS})
target_link_libraries(test_trace dl kv_message protobuf-lite)
