#include "treewidth_stdio_runner.h"

void TreewidthStdioRunner::run() {
    Graph inputGraph;
    inputGraph.load(std::cin);

    TreeDecomposition td;
    td.load(std::cin);
//    td.printTree(std::cout);

    td.convertToNice(inputGraph);
//    td.printTree(std::cout);

    std::unique_ptr<Solver> solver;
    if (td.getWidth() > 16) {
        solver = std::make_unique<DreyfusWagner>(DreyfusWagner(inputGraph, td));
    } else {
        if (inputGraph.getTerminals().size() < 14) {
            solver = std::make_unique<DreyfusWagner>(DreyfusWagner(inputGraph, td));
        } else {
            solver = std::make_unique<ReduceDPSolver>(ReduceDPSolver(inputGraph, td));

        }
    }
    Graph solution = solver->solve();
}
