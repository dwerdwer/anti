cmake_minimum_required(VERSION 2.6)

#cmake_policy(SET CMP0015 OLD)

set (CMAKE_C_COMPILER ${CMAKE_CXX_COMPILER})

#set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fvisibility=hidden -Wall")

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -Wall -fPIC -Wl,-rpath=./ -fvisibility=hidden -Wno-unused -Wno-deprecated-declarations -L../../../lib")

include_directories(
	${PROJECT_SOURCE_DIR}/include/ 
	${PROJECT_SOURCE_DIR}/../../src/common/
    ${PROJECT_SOURCE_DIR}/../kv_engine/inc/
    ${PROJECT_SOURCE_DIR}/../../src/rpc_client/src/
    ${PROJECT_SOURCE_DIR}/../../src/ipc_server/src/
    ${PROJECT_SOURCE_DIR}/../../src/rpc_message/src/)

set(COMMON_SOURCES ${PROJECT_SOURCE_DIR}/implements.cxx)

# set (EDR_MONITOR_SOURCES ${EDR_MONITOR_COMMON_SOURCES} ${COMMON_SOURCES}src/)

add_library(rpcsrv SHARED ${COMMON_SOURCES})

target_link_libraries(rpcsrv ipc_server rpc_message protobuf-lite module_data)

add_custom_command(TARGET rpcsrv POST_BUILD COMMAND cp -vf $<TARGET_FILE:rpcsrv> ${PROJECT_SOURCE_DIR}/../../lib)

