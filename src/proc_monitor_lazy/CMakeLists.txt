cmake_minimum_required(VERSION 2.6)

set (CMAKE_C_COMPILER ${CMAKE_CXX_COMPILER}) 
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fvisibility=hidden -Wall")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++98 -g -fvisibility=hidden -Wall -fPIC -Wno-unused")

include_directories(src inc private_include)

link_directories(${PROJECT_SOURCE_DIR}/lib)

aux_source_directory(src PROC_MONITOR_COMMON_SOURCES)
set (PROC_MONITOR_SOURCES ${PROC_MONITOR_COMMON_SOURCES} ${COMMON_SOURCES} src/)
add_library(proc_monitor_sdk SHARED ${PROC_MONITOR_SOURCES})

target_link_libraries(proc_monitor_sdk pthread)

add_custom_command(TARGET proc_monitor_sdk
        POST_BUILD                                                            
            COMMAND cp -vf $<TARGET_FILE:proc_monitor_sdk> ${PROJECT_SOURCE_DIR}/../../lib)
