cmake_minimum_required(VERSION 3.9)
project(pace2018)

set(CMAKE_CXX_STANDARD 14)

add_executable(pace2018 src/main.cpp src/structures/graph.cpp src/structures/graph.h src/utility/stdio_runner.cpp src/utility/stdio_runner.h src/structures/tree_decomposition.cpp src/structures/tree_decomposition.h)
target_include_directories(pace2018 PRIVATE ${CMAKE_SOURCE_DIR}/src)
