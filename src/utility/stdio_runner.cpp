#include "stdio_runner.h"

void StdioRunner::run() {
    // Graph inputGraph;
    // inputGraph.load(std::cin);

    TreeDecomposition td;
    td.load(std::cin);
    td.printTree(std::cout);
    std::cout << " ----- " << std::endl;

    td.convertToNice();
    td.printTree(std::cout);
}
