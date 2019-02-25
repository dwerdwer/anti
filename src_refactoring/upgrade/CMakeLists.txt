cmake_minimum_required(VERSION 2.6)

set (CMAKE_C_COMPILER ${CMAKE_CXX_COMPILER})
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -DDEBUG -g -fvisibility=hidden -Wall -fPIC -Wno-unused -L../../../lib")

include_directories(${PROJECT_SOURCE_DIR}/src/ ${PROJECT_SOURCE_DIR}/inc ${PROJECT_SOURCE_DIR}/../kv_engine/inc/ ${PROJECT_SOURCE_DIR}/../rpcsrv/include/ ${PROJECT_SOURCE_DIR}/../../src/common/ ${PROJECT_SOURCE_DIR}/../../src/rpc_message/src/)

aux_source_directory(${PROJECT_SOURCE_DIR}/src/ LOCAL_SOURCES)
# set (LOCAL_SOURCES ${LOCAL_SOURCES} ${COMMON_SOURCES})
add_library(upgrade_plugin SHARED ${LOCAL_SOURCES})

target_link_libraries(upgrade_plugin module_data)

add_custom_command(TARGET upgrade_plugin
        POST_BUILD
            COMMAND cp -vf $<TARGET_FILE:upgrade_plugin> ${PROJECT_SOURCE_DIR}/../../lib)
