cmake_minimum_required(VERSION 2.6)                                                                                  

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -Wall")

include_directories(include ../common ../common/sqlite inc)

set (FILE_DIGEST_SQLITE_SOURCES src/file_digest_sqlite.c 
    src/file_digest_interfaces.c ../common/sqlite/sqlite_cmd.c)
link_directories(${PROJECT_SOURCE_DIR}/../../lib)

add_library(file_digest SHARED ${FILE_DIGEST_SQLITE_SOURCES})

target_link_libraries(file_digest ssl crypto pthread file_digest sqlite module_data)

add_custom_command(TARGET file_digest
    POST_BUILD
    COMMAND cp -vf $<TARGET_FILE:file_digest> ${PROJECT_SOURCE_DIR}/../../lib)

