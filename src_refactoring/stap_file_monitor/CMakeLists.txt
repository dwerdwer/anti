cmake_minimum_required(VERSION 2.6)

set (CMAKE_C_COMPILER ${CMAKE_CXX_COMPILER})
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -DDEBUG -g -fPIC -std=c++11 -Wall -Wno-unused-function -L../../../lib")

include_directories(
    ${PROJECT_SOURCE_DIR}/inc/
    ${PROJECT_SOURCE_DIR}/src/
    ${PROJECT_SOURCE_DIR}/../../src/common/
    ${PROJECT_SOURCE_DIR}/../../src/rpc_message/src/
    ${PROJECT_SOURCE_DIR}/../kv_engine/inc/
    ${PROJECT_SOURCE_DIR}/../rpcsrv/include/
    ${PROJECT_SOURCE_DIR}/../file_monitor_sdk/inc/)

aux_source_directory(${PROJECT_SOURCE_DIR}/src/ LOCAL_SOURCES)

add_library(file_monitor SHARED ${LOCAL_SOURCES})

target_link_libraries(file_monitor module_data pthread file_monitor_sdk jsoncpp)

add_custom_command(TARGET file_monitor POST_BUILD
    COMMAND cp -vf $<TARGET_FILE:file_monitor> ${PROJECT_SOURCE_DIR}/../../lib)

