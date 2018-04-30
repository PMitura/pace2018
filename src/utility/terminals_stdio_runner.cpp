#include "terminals_stdio_runner.h"

void TerminalsStdioRunner::run() {
    Graph inputGraph;
    inputGraph.load(std::cin);

    DreyfusWagner solver(inputGraph, TreeDecomposition());
    Graph solution = solver.solve();
}
