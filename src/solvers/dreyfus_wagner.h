#ifndef PACE2018_DREYFUS_WAGNER_H
#define PACE2018_DREYFUS_WAGNER_H

#include <climits>
#include <cstring>
#include <iostream>
#include <queue>

#include "solvers/solver.h"

class DreyfusWagner : public Solver {
public:
    DreyfusWagner(const Graph &inputGraph, const TreeDecomposition &niceDecomposition)
            : Solver(inputGraph, niceDecomposition) {
        INFTY = (UINT_MAX >> 1u) - 10;
        if (INFTY < inputGraph.getEdgeWeightSum()) {
            // insufficient data type for the input graph weights
            exit(1);
        }
    }

    virtual Graph solve() override;

private:
    void backtrack(unsigned subset, int root, std::vector<std::pair<int, int>> &tree);

    unsigned ** dp, ** dp_par;
    int * parent, * closed;
    unsigned * dist;
    unsigned INFTY;
};


#endif //PACE2018_DREYFUS_WAGNER_H
