cmake_minimum_required(VERSION 3.10)
project(REACTOR)
set(CMAKE_BUILD_TYPE Debug)
include_directories(${PROJECT_SOURCE_DIR}/include/config)
include_directories(${PROJECT_SOURCE_DIR}/include/net)
include_directories(${PROJECT_SOURCE_DIR}/include/protocol)
include_directories(${PROJECT_SOURCE_DIR}/include/core)
include_directories(${PROJECT_SOURCE_DIR}/include/thread)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_subdirectory(${PROJECT_SOURCE_DIR}/src)


