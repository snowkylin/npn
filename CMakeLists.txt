cmake_minimum_required(VERSION 3.16)
project(npn)

set(CMAKE_CXX_STANDARD 14)

#add_executable(npn_exe npn/npn.cpp)
add_library(npn MODULE npn/npn.cpp)