cmake_minimum_required(VERSION 3.8)
project(ev)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

add_subdirectory(ev/reactor)
add_subdirectory(ev/test)
