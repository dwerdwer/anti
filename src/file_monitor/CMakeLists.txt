cmake_minimum_required(VERSION 2.6)

set (CMAKE_C_COMPILER ${CMAKE_CXX_COMPILER}) 

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden -g -fPIC -Wall")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -g -DDEBUG -fPIC -std=c++11 -Wall -Wno-unused-function")

include_directories(inc src ../common ../stap_monitor/inc)

link_directories(${PROJECT_SOURCE_DIR}/../../lib)

aux_source_directory(src MONITOR_COMMON_SOURCES)
set (MONITOR_SOURCES ${MONITOR_COMMON_SOURCES} src/interface_monitor.cpp 
    src/module_file_monitor_impl.cpp src/file_system_monitor.cpp)

add_library(file_monitor SHARED ${MONITOR_SOURCES})
target_link_libraries(file_monitor module_data pthread)

add_custom_command(TARGET file_monitor POST_BUILD
    COMMAND cp -vf $<TARGET_FILE:file_monitor> ${PROJECT_SOURCE_DIR}/../../lib)

