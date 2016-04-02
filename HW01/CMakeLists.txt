cmake_minimum_required(VERSION 3.0)
project(HW01 C)

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})
set(CMAKE_C_COMPILER /usr/bin/gcc)
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -std=c99 -g -Wall")


add_subdirectory(linked_list)
include_directories(linked_list)
include_directories(build/include)
add_subdirectory(src)
