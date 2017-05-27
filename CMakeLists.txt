cmake_minimum_required(VERSION 3.2)
project(malt)

set(CMAKE_CXX_STANDARD 14)

add_library(track_ptr INTERFACE)
target_sources(track_ptr INTERFACE track_ptr.hpp)

add_executable(track_test t_ptr_test.cpp)

target_link_libraries(track_test PUBLIC track_ptr)