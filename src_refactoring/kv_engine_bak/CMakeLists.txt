cmake_minimum_required(VERSION 2.6)

project(kv_engine_client_sdk)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -Wall -fPIC -L../../../lib")

include_directories(${PROJECT_SOURCE_DIR}/../../src/rpc_client/src ${PROJECT_SOURCE_DIR}/../../src/rpc_message/src/ ${PROJECT_SOURCE_DIR}/../../src/common ${PROJECT_SOURCE_DIR}/inc/)

aux_source_directory(${PROJECT_SOURCE_DIR}/src/ LOCAL_SOURCES)

set(ALL_SOURCES ${LOCAL_SOURCES})

add_library(kv_engine_client_sdk SHARED ${ALL_SOURCES})

target_link_libraries(kv_engine_client_sdk pthread rpc_client)

add_custom_command(TARGET kv_engine_client_sdk POST_BUILD COMMAND
    cp -vf $<TARGET_FILE:kv_engine_client_sdk> ${PROJECT_SOURCE_DIR}/../../lib)
