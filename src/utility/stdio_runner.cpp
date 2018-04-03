#include "stdio_runner.h"

void StdioRunner::run() {
    Graph inputGraph;
    inputGraph.load(std::cin);

    TreeDecomposition td;
    td.load(std::cin);
    // td.printTree(std::cout);
    // std::cout << " ----- " << std::endl;

    td.convertToNice(inputGraph);
    td.addNodeEverywhere(inputGraph.getTerminals()[0]);
    // td.printTree(std::cout);

    ReduceDPSolver solver(inputGraph, td);
    Graph solution = solver.solve();
}
