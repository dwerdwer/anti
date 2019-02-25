cmake_minimum_required(VERSION 2.6)

set (CMAKE_C_COMPILER ${CMAKE_CXX_COMPILER})
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++98 -g -fvisibility=hidden -Wall -fPIC -Wno-unused -L../../../lib")

include_directories(${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/inc ${PROJECT_SOURCE_DIR}/private_include ${PROJECT_SOURCE_DIR}/../common ${PROJECT_SOURCE_DIR}/../rpcsrv/include)

aux_source_directory(src PROC_MONITOR_SOURCES)
set (PROC_MONITOR_SOURCES ${PROC_MONITOR_SOURCES} ${COMMON_SOURCES})
add_library(proc_monitor_sdk SHARED ${PROC_MONITOR_SOURCES})

target_link_libraries(proc_monitor_sdk pthread module_data)

add_custom_command(TARGET proc_monitor_sdk
        POST_BUILD
            COMMAND cp -vf $<TARGET_FILE:proc_monitor_sdk> ${PROJECT_SOURCE_DIR}/../../lib)
