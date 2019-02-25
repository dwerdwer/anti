cmake_minimum_required(VERSION 2.6)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -std=c++11 -g -Wall")

include_directories(usb_monitor ../common)

link_directories(${PROJECT_SOURCE_DIR}/../../lib)

set (MONITOR_SOURCES usb_monitor/interface.cpp)

add_library(usb_monitor SHARED ${MONITOR_SOURCES})
target_link_libraries(usb_monitor jsoncpp module_data)

add_custom_command(TARGET usb_monitor
    POST_BUILD
    COMMAND cp -vf $<TARGET_FILE:usb_monitor> ${PROJECT_SOURCE_DIR}/../../lib)


