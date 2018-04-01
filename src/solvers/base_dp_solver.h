#ifndef PACE2018_BASEDPSOLVER_H
#define PACE2018_BASEDPSOLVER_H

#include <algorithm>
#include <climits>
#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

#include "solvers/solver.h"
#include "structures/graph.h"
#include "structures/tree_decomposition.h"
#include "utility/helpers.h"
#include "utility/partitioner.h"
#include "utility/partition_mergers.h"

class BaseDPSolver : public Solver {
public:
    BaseDPSolver(const Graph &inputGraph, const TreeDecomposition &niceDecomposition)
            : Solver(inputGraph, niceDecomposition),
              globalTerminal(-1) {
        INFTY = (UINT_MAX >> 1u) - 10;
        if (INFTY < inputGraph.getEdgeWeightSum()) {
            // insufficient data type for the input graph weights
            exit(1);
        }
    }

    Graph solve() override;

private:
    unsigned solveInstance(int treeNode, unsigned int subset, uint64_t partition);
    void backtrack(int treeNode, int subset, uint64_t partition);
    void initializeDP();

    unsigned resolveIntroNode(TreeDecomposition::Node &node,
                              int treeNode, unsigned subset, uint64_t partition);
    unsigned resolveForgetNode(TreeDecomposition::Node &node,
                               int treeNode, unsigned int subset, uint64_t partition);
    unsigned resolveJoinNode(TreeDecomposition::Node &node,
                             int treeNode, unsigned int subset, uint64_t partition);
    unsigned resolveEdgeNode(TreeDecomposition::Node &node,
                             int treeNode, unsigned int subset, uint64_t partition);
    unsigned resolveLeafNode(unsigned int subset);

    void printDPState(TreeDecomposition::Node &node,
                      int treeNode, unsigned int subset, uint64_t partition);

    std::vector<std::vector<std::unordered_map<uint64_t, unsigned>>> dpCache;
    std::vector<std::vector<std::unordered_map<uint64_t, std::vector<uint64_t>>>> dpBacktrack;
    std::vector<std::pair<int, int>> resultEdges;
    int globalTerminal;
    unsigned INFTY;
};


#endif //PACE2018_BASEDPSOLVER_H
