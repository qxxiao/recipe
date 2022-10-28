cmake_minimum_required(VERSION 3.0)
set(CMAKE_CXX_COMPILER "/usr/local/bin/g++")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -w -std=c++17 -pthread")


project(lockfreequeue)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
ADD_DEFINITIONS(-DDEBUG) #注意一定要有-D

add_subdirectory(fix_aba)
add_subdirectory(normal)