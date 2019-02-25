cmake_minimum_required(VERSION 2.6)

set (CMAKE_C_COMPILER ${CMAKE_CXX_COMPILER}) 
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fvisibility=hidden -Wall")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -fvisibility=hidden -Wall")

include_directories(src ../common)

link_directories(${PROJECT_SOURCE_DIR}/../../lib)

set (LOGGER_SOURCES src/logger.cxx)
add_library(logger SHARED ${LOGGER_SOURCES})
target_link_libraries(logger pthread module_data)

add_custom_command(TARGET logger
    POST_BUILD
    COMMAND cp -vf $<TARGET_FILE:logger> ${PROJECT_SOURCE_DIR}/../../lib)



