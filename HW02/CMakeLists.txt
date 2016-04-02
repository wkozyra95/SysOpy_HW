cmake_minimum_required(VERSION 3.0)
project(HW02 C)

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})
set(CMAKE_C_COMPILER /usr/bin/gcc)
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -std=c99 -g -Wall")


add_subdirectory(task1)
add_subdirectory(task2)
add_subdirectory(task3)

