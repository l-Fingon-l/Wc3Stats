cmake_minimum_required(VERSION 3.20)
project(Wc3Stats)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(Local)
add_executable(preprocess Local/preprocess/preprocess.cpp)
add_executable(serialization Local/serialization/serialization.cpp Local/test.cpp)
