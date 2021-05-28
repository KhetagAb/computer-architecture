cmake_minimum_required(VERSION 3.17)
project(hm5)

set(CMAKE_CXX_FLAGS "-std=c++17 -fopenmp")

add_executable(hm5 main.cpp)