cmake_minimum_required(VERSION 3.21)
project(test C)

set(CMAKE_C_STANDARD 99)

add_executable(run_tests sme.c libs/CuTest.c)