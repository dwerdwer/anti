cmake_minimum_required(VERSION 2.6)

project(uploader_factory)

set (CMAKE_C_FLAGS ${CMAKE_CXX_FLAGS})
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -fPIC -L../../../lib")

include_directories(inc src http_src ftp_src ../common)

set (COMM_SOURCES src/zip.cpp src/uploader_factory.cpp)
set (HTTP_SOURCES http_src/http_uploader.cpp http_src/upload_report.cpp)
set (FTP_SOURCES ftp_src/ftp_uploader.cpp ftp_src/upload_report.cpp)

set (UPLOAD_SOURCES ${COMM_SOURCES} ${HTTP_SOURCES} ${FTP_SOURCES})

add_library(uploader_factory SHARED ${UPLOAD_SOURCES})

target_link_libraries(uploader_factory curl pthread)

add_custom_command(TARGET uploader_factory POST_BUILD
    COMMAND cp -vf $<TARGET_FILE:uploader_factory> ${PROJECT_SOURCE_DIR}/../../lib)
