cmake_minimum_required(VERSION 2.6)
#cmake_policy(VERSION 2.8.9)
project(file_reporter)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -DDEBUG -g -Wall -fvisibility=hidden ")

include_directories(reporter inc ../common ../network ../center_agent/kv_message/src)

set (REPORTER_SOURCES report/interface_report.cpp report/zip.cpp report/upload_utils.cpp)

link_directories( ${PROJECT_SOURCE_DIR}/../../lib)
add_library(file_reporter SHARED ${REPORTER_SOURCES})
target_link_libraries(file_reporter curl sqlite module_data rt)

add_custom_command(TARGET file_reporter POST_BUILD
    COMMAND cp -vf $<TARGET_FILE:file_reporter> ${PROJECT_SOURCE_DIR}/../../lib)



