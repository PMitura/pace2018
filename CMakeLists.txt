cmake_minimum_required(VERSION 3.9)
project(pace2018)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -pedantic -Wno-narrowing -Wno-vla -Wno-maybe-uninitialized -Ofast")

add_executable(pace2018-problemB src/treewidth_main.cpp src/structures/graph.cpp src/structures/graph.h src/utility/treewidth_stdio_runner.cpp src/utility/treewidth_stdio_runner.h src/structures/tree_decomposition.cpp src/structures/tree_decomposition.h src/utility/helpers.h src/solvers/solver.h src/solvers/base_dp_solver.cpp src/solvers/base_dp_solver.h src/utility/partitioner.cpp src/utility/partitioner.h src/utility/partition_mergers.cpp src/utility/partition_mergers.h src/structures/union_find.h src/solvers/table_dp_solver.cpp src/solvers/table_dp_solver.h src/structures/cut_matrix.cpp src/structures/cut_matrix.h src/solvers/reduce_dp_solver.cpp src/solvers/reduce_dp_solver.h src/solvers/dreyfus_wagner.cpp src/solvers/dreyfus_wagner.h src/utility/terminals_stdio_runner.cpp src/utility/terminals_stdio_runner.h)
add_executable(pace2018-problemA src/terminals_main.cpp src/structures/graph.cpp src/structures/graph.h src/utility/treewidth_stdio_runner.h src/structures/tree_decomposition.cpp src/structures/tree_decomposition.h src/utility/helpers.h src/solvers/solver.h src/solvers/base_dp_solver.cpp src/solvers/base_dp_solver.h src/utility/partitioner.cpp src/utility/partitioner.h src/utility/partition_mergers.cpp src/utility/partition_mergers.h src/structures/union_find.h src/solvers/table_dp_solver.cpp src/solvers/table_dp_solver.h src/structures/cut_matrix.cpp src/structures/cut_matrix.h src/solvers/reduce_dp_solver.cpp src/solvers/reduce_dp_solver.h src/solvers/dreyfus_wagner.cpp src/solvers/dreyfus_wagner.h src/utility/terminals_stdio_runner.cpp src/utility/terminals_stdio_runner.h)
target_include_directories(pace2018-problemB PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(pace2018-problemA PRIVATE ${CMAKE_SOURCE_DIR}/src)
