cmake_minimum_required(VERSION 2.6)
#cmake_policy(VERSION 2.8.9)

set (CMAKE_C_COMPILER ${CMAKE_CXX_COMPILER}) 

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fvisibility=hidden -Wall -L${PROJECT_SOURCE_DIR}/../../lib")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -fvisibility=hidden -Wall -L${PROJECT_SOURCE_DIR}/../../lib")

include_directories(common ../common)

#link_directories(../lib ../../common/lib)
#This is for cmake 2.8.1 or later.
#link_directories(lib ../common/lib)
#link_directories(${PROJECT_SOURCE_DIR}/../../lib)

set(COMMON_SOURCES ../common/utils/utils_bits.c ../common/utils/utils_library.c)
aux_source_directory(common SCHEDULER_COMMON_SOURCES)
set (CBD_SCHEDULER_SOURCES ${SCHEDULER_COMMON_SOURCES} ${COMMON_SOURCES}
    cbd_scheduler/cbd_scheduler.cxx 
    cbd_scheduler/cbd_scheduler_interfaces.cxx)
add_library(cbd_scheduler SHARED ${CBD_SCHEDULER_SOURCES})
target_link_libraries(cbd_scheduler pthread dl expat module_data)

add_custom_command(TARGET cbd_scheduler
    POST_BUILD
    COMMAND cp -vf $<TARGET_FILE:cbd_scheduler> ${PROJECT_SOURCE_DIR}/../../lib)


