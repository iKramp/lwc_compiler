cmake_minimum_required(VERSION 3.22)
project(lwc_compiler)

set(CMAKE_CXX_STANDARD 20)

add_executable(lwc_compiler main.cpp tokenize/tokenize.cpp tokenize/tokenize.h parse/parse.cpp parse/parse.h)
