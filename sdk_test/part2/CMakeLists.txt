cmake_minimum_required(VERSION 2.6)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wconversion")

include_directories(${PROJECT_SOURCE_DIR}/src/tinyxml)

aux_source_directory(${PROJECT_SOURCE_DIR}/src RUN_TEST_SOURCE)

aux_source_directory(${PROJECT_SOURCE_DIR}/src/tinyxml TINYXML_SOURCE)

add_executable(run_modify_load ${RUN_TEST_SOURCE} ${TINYXML_SOURCE})

#target_link_libraries(run_modify_load tinyxml)

