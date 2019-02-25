cmake_minimum_required(VERSION 3.5)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -Wconversion -Wall")

include_directories(../inc)
link_directories(${PROJECT_SOURCE_DIR}/../../../lib)

set (TEST_SOURCES main_test.cpp)
#set (TEST_SOURCES simple_run.cpp)

add_executable(run_test ${TEST_SOURCES})

target_link_libraries(run_test file_monitor_sdk pthread)



