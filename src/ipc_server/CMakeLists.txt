cmake_minimum_required(VERSION 2.6)
project(ipc_server)

#cmake_policy(VERSION 2.6)
cmake_policy(SET CMP0015 OLD)

#Force to use CXX compiler to compile C source code.
set (CMAKE_C_COMPILER ${CMAKE_CXX_COMPILER})

#TODO: remove '-g', use '-DCMAKE_BUILD_TYPE=DEBUG' instead.
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -fPIC -g -Wall -Wconversion -fvisibility=hidden -L../../../lib")
#set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wconversion")
#set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wconversion --coverage")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC -g -Wall -Wconversion -fvisibility=hidden -L../../../lib")

SET (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../../lib)
SET (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)

include_directories(include src ../common/include ../common/src ../common ../rpc_message/src)

## ipc_server  Dynamic library
set (IPC_SERVER_SOURCES src/ipc_server.cpp
    src/libwebsockets_server_eventloop.c src/server_http.c
    src/libwebsockets_server_module_defines.cxx
    ../common/src/queue.c 
    ../common/src/thread_pool.c 
    ../common/src/shared_ptr.c
    ../common/src/utils/utils_integer.c ../common/src/utils/utils_signal.c 
    ../common/src/utils/utils_time.c ../common/src/utils/utils_network.c
        src/libwebsockets_server_module_defines.cxx)
add_library(ipc_server_shared SHARED ${IPC_SERVER_SOURCES})
set_target_properties(ipc_server_shared
    PROPERTIES
    OUTPUT_NAME ipc_server)

target_link_libraries(ipc_server_shared rpc_message protobuf-lite websockets ssl crypto pthread )

#add_custom_command(TARGET ipc_server_shared
#    POST_BUILD
#    COMMAND cp -vf $<TARGET_FILE:ipc_server_shared> ${PROJECT_SOURCE_DIR}/../../lib)

#ADD_SUBDIRECTORY(test)
###
