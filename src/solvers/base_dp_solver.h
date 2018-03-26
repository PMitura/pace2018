#ifndef PACE2018_BASEDPSOLVER_H
#define PACE2018_BASEDPSOLVER_H

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>

#include "solvers/solver.h"
#include "structures/graph.h"
#include "structures/tree_decomposition.h"
#include "utility/helpers.h"
#include "utility/partitioner.h"

class BaseDPSolver : public Solver {
public:
    BaseDPSolver(const Graph &inputGraph, const TreeDecomposition &niceDecomposition)
            : Solver(inputGraph, niceDecomposition),
              dpCache(nullptr), dpBacktrack(nullptr),
              globalTerminal(-1) {}

    Graph solve() override;

private:
    const int INFTY = 1 << 29;

    int solveInstance(int treeNode, int subset, uint64_t partition);
    void backtrack(int treeNode, int subset, uint64_t partition);
    void initializeDP();
    void cleanupDP();

    int resolveIntroNode(TreeDecomposition::Node &node,
                         int treeNode, int subset, uint64_t partition);
    int resolveForgetNode(TreeDecomposition::Node &node,
                          int treeNode, int subset, uint64_t partition);
    int resolveJoinNode(TreeDecomposition::Node &node,
                        int treeNode, int subset, uint64_t partition);
    int resolveEdgeNode(TreeDecomposition::Node &node,
                        int treeNode, int subset, uint64_t partition);
    int resolveLeafNode(int subset);

    void printDPState(TreeDecomposition::Node &node,
                      int treeNode, int subset, uint64_t partition);

    std::unordered_map<uint64_t, int> ** dpCache;
    std::unordered_map<uint64_t, std::vector<uint64_t>> ** dpBacktrack;
    std::vector<std::pair<int, int>> resultEdges;
    int globalTerminal;
};


#endif //PACE2018_BASEDPSOLVER_H
