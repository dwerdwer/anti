cmake_minimum_required(VERSION 2.6)

set (CMAKE_C_COMPILER ${CMAKE_CXX_COMPILER})
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++98 -g -Wall -fPIC -fvisibility=hidden -Wno-unused -L../../../lib")

include_directories(${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/inc ${PROJECT_SOURCE_DIR}/private_include ${PROJECT_SOURCE_DIR}/../common ${PROJECT_SOURCE_DIR}/../rpcsrv/include)

aux_source_directory(src NET_MONITOR_SOURCES)
set (NET_MONITOR_SOURCES ${NET_MONITOR_SOURCES} ${COMMON_SOURCES})
add_library(net_monitor_sdk SHARED ${NET_MONITOR_SOURCES})

target_link_libraries(net_monitor_sdk pthread pcap module_data)

add_custom_command(TARGET net_monitor_sdk POST_BUILD
    COMMAND cp -vf $<TARGET_FILE:net_monitor_sdk> ${PROJECT_SOURCE_DIR}/../../lib)
