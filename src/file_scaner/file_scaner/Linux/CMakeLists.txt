cmake_minimum_required(VERSION 2.6)
	
set (CMAKE_C_COMPILER ${CMAKE_CXX_COMPILER})

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -Wall -fPIC ")

include_directories(file_scaner ../../../common ../Linux ../AntiVirEngine/Interface ./Include ../../../center_agent/kv_message/src)

aux_source_directory(./ FILE_SCANER_SOURCES)

link_directories(${PROJECT_SOURCE_DIR}/../../../../lib)

add_library(file_scaner SHARED ${FILE_SCANER_SOURCES})
target_link_libraries(file_scaner pthread m rt module_data avpkmgr avscanep avfrec avemgr)

add_custom_command(TARGET file_scaner POST_BUILD
	COMMAND cp -vf $<TARGET_FILE:file_scaner> ${PROJECT_SOURCE_DIR}/../../../../lib)
