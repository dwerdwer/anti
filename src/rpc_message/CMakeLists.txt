cmake_minimum_required(VERSION 2.6)

PROJECT (rpc_message)

set(LIBRPCMSG_SRCS src/RpcMessage.cpp src/RpcMessage.pb.cc src/Utils.cpp)

include_directories(SYSTEM  "../common")

add_library (rpc_message STATIC ${LIBRPCMSG_SRCS})

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -Wall -fPIC")

add_custom_command(TARGET rpc_message
    POST_BUILD
    COMMAND cp -vf $<TARGET_FILE:rpc_message> ${PROJECT_SOURCE_DIR}/../../lib/)

