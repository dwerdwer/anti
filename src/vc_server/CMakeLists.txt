cmake_minimum_required(VERSION 2.6)

set (CMAKE_C_COMPILER ${CMAKE_CXX_COMPILER}) 
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -DDEBUG -fvisibility=hidden -Wall")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -DDEBUG -std=c++11 -fvisibility=hidden -Wall -Wno-unused")

include_directories(../common ../rpc_message/src/ ../common/sqlite/ inc)

link_directories(${PROJECT_SOURCE_DIR}/../../lib)

set (VC_SERVER_SOURCES src/virus_check_server.cxx 
    src/access_mysql.cxx src/vc_server_function.cxx)

add_library(vc_server SHARED ${VC_SERVER_SOURCES})

target_link_libraries(vc_server module_data
        rpcmessage ${PROJECT_SOURCE_DIR}/lib/libipc_server.so)

add_custom_command(TARGET vc_server POST_BUILD
    COMMAND cp -vf $<TARGET_FILE:vc_server> ${PROJECT_SOURCE_DIR}/../../lib)



