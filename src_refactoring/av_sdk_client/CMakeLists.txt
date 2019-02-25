
cmake_minimum_required(VERSION 2.6)

project(antivirus)

#set (DYNAMIC_LINKER 
#    "-Wl,--dynamic-linker,${PROJECT_SOURCE_DIR}/../../lib/ld-linux-x86-64.so.2 -Wl,-rpath,${PROJECT_SOURCE_DIR}/../../../lib")

#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -Wall ${DYNAMIC_LINKER}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -Wall -L../../../lib")

include_directories( ${PROJECT_SOURCE_DIR}/../../src_refactoring/av_sdk/src )

set(Src ${PROJECT_SOURCE_DIR}/src/main.cxx)

add_executable(antivirus ${Src})

target_link_libraries(antivirus avxsdk pthread)



