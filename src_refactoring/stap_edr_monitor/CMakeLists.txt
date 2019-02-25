cmake_minimum_required(VERSION 2.6)

set (CMAKE_C_COMPILER ${CMAKE_CXX_COMPILER})
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -fPIC -fvisibility=hidden -g -L../../../lib")

include_directories(${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/../common ${PROJECT_SOURCE_DIR}/../common/sqlite/
    ${PROJECT_SOURCE_DIR}/../../src/usb_monitor/usb_monitor/ ${PROJECT_SOURCE_DIR}/../stap_proc_monitor/inc ${PROJECT_SOURCE_DIR}/../stap_net_monitor/inc ${PROJECT_SOURCE_DIR}/../upload_factory/inc)

set(COMMON_SOURCES ${PROJECT_SOURCE_DIR}/../common/sqlite/sqlite_cmd.c)
aux_source_directory(${PROJECT_SOURCE_DIR}/src EDR_MONITOR_SOURCES)
set (EDR_MONITOR_SOURCES ${EDR_MONITOR_SOURCES} ${COMMON_SOURCES})
add_library(sysinfo SHARED ${EDR_MONITOR_SOURCES})

target_link_libraries(sysinfo pthread ssl jsoncpp net_monitor_sdk proc_monitor_sdk module_data crypto sqlite uploader_factory)

add_custom_command(TARGET sysinfo POST_BUILD COMMAND 
    cp -vf $<TARGET_FILE:sysinfo> ${PROJECT_SOURCE_DIR}/../../lib)
