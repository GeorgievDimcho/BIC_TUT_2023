cmake_minimum_required(VERSION 3.10.0)
project(ExampleProject C CXX)
include_directories(src)
include_directories(test)
add_library(generator_library src/project1group1.c)

#include(mathcalls.h)
add_executable(proj1_binary src/project1group1.c mathcalls.h)
add_executable(unit_tests_binary test/unit_tests.c)


