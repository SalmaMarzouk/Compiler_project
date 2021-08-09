cmake_minimum_required(VERSION 3.19)
project(compiler)

set(CMAKE_CXX_STANDARD 20)

add_executable(compiler main.cpp parser.h parser.cpp analyzer.cpp analyzer.h DisjSet.h)