cmake_minimum_required(VERSION 2.6)                                                                                  

set (CMAKE_C_COMPILER ${CMAKE_CXX_COMPILER}) 
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -Wall") 

include_directories(include ../common inc)

set (CAPTURE_PACKET_SOURCES src/capture_network_packet.cxx  src/dns.c)

link_directories(${PROJECT_SOURCE_DIR}/../../lib)

add_library(capture_packet SHARED ${CAPTURE_PACKET_SOURCES})
target_link_libraries(capture_packet pthread pcap)

add_custom_command(TARGET capture_packet 
    POST_BUILD
    COMMAND cp -vf $<TARGET_FILE:capture_packet> ${PROJECT_SOURCE_DIR}/../../lib/)

