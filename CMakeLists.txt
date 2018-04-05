cmake_minimum_required(VERSION 3.9)
project(pace2018)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -pedantic -Wno-narrowing -Wno-vla")
set(CMAKE_CXX_FLAGS_DEBUG "-ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -march=native -ffast-math")

add_executable(pace2018 src/main.cpp src/structures/graph.cpp src/structures/graph.h src/utility/stdio_runner.cpp src/utility/stdio_runner.h src/structures/tree_decomposition.cpp src/structures/tree_decomposition.h src/utility/helpers.h src/solvers/solver.h src/solvers/base_dp_solver.cpp src/solvers/base_dp_solver.h src/utility/partitioner.cpp src/utility/partitioner.h src/utility/partition_mergers.cpp src/utility/partition_mergers.h src/structures/union_find.h src/solvers/table_dp_solver.cpp src/solvers/table_dp_solver.h src/structures/cut_matrix.cpp src/structures/cut_matrix.h src/solvers/reduce_dp_solver.cpp src/solvers/reduce_dp_solver.h)
target_include_directories(pace2018 PRIVATE ${CMAKE_SOURCE_DIR}/src)

# google test conf
add_executable(pace2018_gtest src/structures/graph.cpp src/structures/graph.h src/utility/stdio_runner.cpp src/utility/stdio_runner.h src/structures/tree_decomposition.cpp src/structures/tree_decomposition.h tests/tests_main.cpp tests/suites/stucture_correctness.cpp src/utility/helpers.h tests/suites/helpers_correctness.cpp src/solvers/solver.h src/solvers/base_dp_solver.cpp src/solvers/base_dp_solver.h src/utility/partitioner.cpp src/utility/partitioner.h tests/suites/partitioner_correctness.cpp src/utility/partition_mergers.cpp src/utility/partition_mergers.h tests/suites/mergers_correctness.cpp src/structures/union_find.h src/solvers/table_dp_solver.cpp src/solvers/table_dp_solver.h src/structures/cut_matrix.cpp src/structures/cut_matrix.h src/solvers/reduce_dp_solver.cpp src/solvers/reduce_dp_solver.h)
target_include_directories(pace2018_gtest PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(pace2018_gtest PRIVATE ${CMAKE_SOURCE_DIR}/tests)
# beware, uses system wide google test framework (not recommended, yet simple for our purposes)
target_link_libraries(pace2018_gtest gtest gtest_main pthread)
