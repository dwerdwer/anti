cmake_minimum_required(VERSION 2.6)


set (CMAKE_C_COMPILER ${CMAKE_CXX_COMPILER})
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden -g -fPIC -Wall")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -g -fPIC -Wall")

include_directories(inc src ../../src/common)

# set (MONITOR_SOURCES src/module_file_monitor_impl.cpp
#     src/file_system_monitor.cpp src/stap_file_monitor.c src/util-string.c)

aux_source_directory(${PROJECT_SOURCE_DIR}/src MONITOR_SOURCES)

add_library(file_monitor_sdk SHARED ${MONITOR_SOURCES})

target_link_libraries(file_monitor_sdk pthread)

add_custom_command(TARGET file_monitor_sdk POST_BUILD
    COMMAND cp -vf $<TARGET_FILE:file_monitor_sdk> ${PROJECT_SOURCE_DIR}/../../lib)

