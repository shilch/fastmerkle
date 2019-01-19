cmake_minimum_required(VERSION 3.13)
project(fastmerkle C)

set(CMAKE_C_STANDARD 99)

find_package(OpenCL REQUIRED)
include_directories(${OpenCL_INCLUDE_DIRS})
link_directories(${OpenCL_LIBRARY})

add_executable(fastmerkle main.c)

target_include_directories (fastmerkle PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries (fastmerkle ${OpenCL_LIBRARY})