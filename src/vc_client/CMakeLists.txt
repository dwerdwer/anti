cmake_minimum_required(VERSION 2.6)

set (CMAKE_C_COMPILER ${CMAKE_CXX_COMPILER}) 
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -DDEBUG -fvisibility=hidden -Wall")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -DDEBUG -std=c++11 -fvisibility=hidden -Wall -Wno-unused")

link_directories(${PROJECT_SOURCE_DIR}/../../lib)

include_directories(../common ../common/sqlite/ inc rpc_client/inc rpc_client/src)

# librpc_client part
aux_source_directory(rpc_client/src LIBRPCCLI_SRCS)

add_library (rpc_client SHARED ${LIBRPCCLI_SRCS})

target_link_libraries(rpc_client rpcmessage protobuf-lite)

add_custom_command(TARGET rpc_client POST_BUILD
    COMMAND cp -vf $<TARGET_FILE:rpc_client> ${PROJECT_SOURCE_DIR}/../../lib)

# vc_client part
set (VC_CLIENT_SOURCES src/virus_check_client.cxx src/vc_client_function.cxx)
add_library(vc_client SHARED ${VC_CLIENT_SOURCES})
target_link_libraries(vc_client module_data rpc_client)

add_custom_command(TARGET vc_client POST_BUILD
    COMMAND cp -vf $<TARGET_FILE:vc_client> ${PROJECT_SOURCE_DIR}/../../lib)



