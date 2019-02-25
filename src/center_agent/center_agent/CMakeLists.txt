cmake_minimum_required(VERSION 2.6)
	
project(center_agent)
#Force to use CXX compiler to compile c source code.
set (CMAKE_C_COMPILER ${CMAKE_CXX_COMPILER})


set (CMAKE_C_COMPILER ${CMAKE_CXX_COMPILER}) 
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fvisibility=hidden -Wall")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,-rpath=./ -std=c++11 -fPIC -fvisibility=hidden -Wall -L../../../../lib -static-libgcc")

include_directories("../../common" "../../network/" "../kv_message/src" "${PROJECT_SOURCE_DIR}/include")

aux_source_directory(../kv_message/src LIBKVMSG_SRCS)

#link_directories(${PROJECT_SOURCE_DIR}/../../../lib)
add_library(kv_message STATIC ${LIBKVMSG_SRCS})
target_link_libraries(kv_message protobuf-lite)

add_custom_command(TARGET kv_message
	POST_BUILD
	COMMAND cp -vf $<TARGET_FILE:kv_message> ${PROJECT_SOURCE_DIR}/../../../lib)



set (COMMON_SRCS )
aux_source_directory(./src SRCS)
set (SRCS ${SRCS} ${COMMON_SRCS})

#add_executable(center_agent ${SRCS})
add_library(center_agent SHARED ${SRCS})

#target_link_libraries(center_agent curl kv_message protobuf-lite jsoncpp uuid module_data)
target_link_libraries(center_agent NetUtils curl kv_message protobuf-lite jsoncpp uuid module_data)
#target_link_libraries(center_agent ${PROJECT_SOURCE_DIR}/lib/libNetUtils.so)
#target_link_libraries(center_agent curl)
#
## link protobuf lib
#target_link_libraries(center_agent ${PROJECT_SOURCE_DIR}/lib/libkv_message.a)
#target_link_libraries(center_agent ${PROJECT_SOURCE_DIR}/lib/libprotobuf-lite.a)
#
## link jsoncpp
#target_link_libraries(center_agent ${PROJECT_SOURCE_DIR}/lib/libjsoncpp.a)

add_custom_command(TARGET center_agent
	POST_BUILD
	COMMAND cp -vf $<TARGET_FILE:center_agent> ${PROJECT_SOURCE_DIR}/../../../lib)

###
############
