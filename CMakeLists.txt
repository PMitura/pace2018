cmake_minimum_required(VERSION 3.9)
project(pace2018)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-Wall -pedantic -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -march=native -ffast-math")

add_executable(pace2018 src/main.cpp src/structures/graph.cpp src/structures/graph.h src/utility/stdio_runner.cpp src/utility/stdio_runner.h src/structures/tree_decomposition.cpp src/structures/tree_decomposition.h)
target_include_directories(pace2018 PRIVATE ${CMAKE_SOURCE_DIR}/src)

# google test conf
add_executable(pace2018_gtest src/structures/graph.cpp src/structures/graph.h src/utility/stdio_runner.cpp src/utility/stdio_runner.h src/structures/tree_decomposition.cpp src/structures/tree_decomposition.h tests/tests_main.cpp tests/suites/stucture_correctness.cpp)
target_include_directories(pace2018_gtest PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(pace2018_gtest PRIVATE ${CMAKE_SOURCE_DIR}/tests)
# beware, uses system wide google test framework (not recommended, yet simple for our purposes)
target_link_libraries(pace2018_gtest gtest gtest_main pthread)
