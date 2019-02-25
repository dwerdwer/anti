cmake_minimum_required(VERSION 2.6)
#cmake_policy(VERSION 2.8.9)

project(main_daemon)

set (DYNAMIC_LINKER "-Wl,--dynamic-linker,../lib/ld-linux-x86-64.so.2 -Wl,-rpath,../lib")

#Force to use CXX compiler to compile c source code.
set (CMAKE_C_COMPILER ${CMAKE_CXX_COMPILER})

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fvisibility=hidden -Wall ${DYNAMIC_LINKER}")

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -fvisibility=hidden -Wall ${DYNAMIC_LINKER}")

include_directories(../common)

#link_directories(../lib ../../common/lib)
#This is for cmake 2.8.1 or later.
link_directories(${PROJECT_SOURCE_DIR}/../../lib)

set (COMMON_SOURCES ../common/utils/utils_process.c ../common/utils/utils_signal.c
    ../common/utils/utils_fd.c ../common/utils/utils_library.c ../common/utils/utils_bits.c ../common/expat/get_configure.c)
aux_source_directory(./ MAIN_DAEMON_SOURCES)
set (MAIN_DAEMON_SOURCES ${MAIN_DAEMON_SOURCES} ${COMMON_SOURCES})
add_executable(main_daemon ${MAIN_DAEMON_SOURCES})
target_link_libraries(main_daemon dl expat pthread)

