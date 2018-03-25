#ifndef PACE2018_SOLVER_H
#define PACE2018_SOLVER_H

#include "structures/graph.h"
#include "structures/tree_decomposition.h"

class Solver {
public:
    Solver(const Graph& inputGraph, const TreeDecomposition &niceDecomposition) :
        graph(inputGraph),
        decomposition(niceDecomposition) {}
    virtual Graph solve() = 0;

protected:
    const Graph &graph;
    const TreeDecomposition &decomposition;
};

#endif //PACE2018_SOLVER_H
