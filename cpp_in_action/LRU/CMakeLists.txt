cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -w")

project(lru)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
ADD_DEFINITIONS(-DDEBUG) #-D

# set -isysroot for macOS
if(APPLE)
    set(CMAKE_OSX_SYSROOT /Library/Developer/CommandLineTools/SDKs/MacOSX12.sdk)
endif()

FILE(GLOB SRCS "*.cc" "*.cpp" "*.h")

add_executable(${PROJECT_NAME} ${SRCS})