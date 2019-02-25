cmake_minimum_required(VERSION 2.6)                                                                                  

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -Wall")

include_directories(include ../common)

set (MODULE_DATA_SOURCES src/module_data.cxx)

add_library(module_data SHARED ${MODULE_DATA_SOURCES})
target_link_libraries(module_data)

add_custom_command(TARGET module_data 
    POST_BUILD
    COMMAND cp -vf $<TARGET_FILE:module_data> ${PROJECT_SOURCE_DIR}/../../lib)

