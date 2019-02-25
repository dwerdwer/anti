cmake_minimum_required(VERSION 2.6)

project(avxsdk)

#set (CMAKE_C_COMPILER ${CMAKE_CXX_COMPILER})

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -Wall -fPIC -L../../../lib")

include_directories(${PROJECT_SOURCE_DIR}/inc 
    ${PROJECT_SOURCE_DIR}/../common 
    ${PROJECT_SOURCE_DIR}/../../src/rpc_client/src 
    ${PROJECT_SOURCE_DIR}/../../src/rpc_message/src)

#include_directories(${PROJECT_SOURCE_DIR}/inc )

#link_directories(./ ../../../lib/)

set(Src ${PROJECT_SOURCE_DIR}/src/antivirus_implements.cxx)

#生成动态库
add_library(avxsdk SHARED ${Src})

target_link_libraries(avxsdk rpc_client pthread)




