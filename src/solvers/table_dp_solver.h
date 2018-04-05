#ifndef PACE2018_TABLE_DP_SOLVER_H
#define PACE2018_TABLE_DP_SOLVER_H

#include <climits>
#include <ctime>
#include <unordered_map>
#include <vector>

#include "solvers/solver.h"
#include "utility/partitioner.h"
#include "utility/partition_mergers.h"

class TableDPSolver : public Solver {
public:
    TableDPSolver(const Graph &inputGraph, const TreeDecomposition &niceDecomposition)
            : Solver(inputGraph, niceDecomposition),
              globalTerminal(-1),
              introTime(0), forgetTime(0), joinTime(0), edgeTime(0), leafTime(0) {
        INFTY = (UINT_MAX >> 1u) - 10;
        if (INFTY < inputGraph.getEdgeWeightSum()) {
            // insufficient data type for the input graph weights
            exit(1);
        }
    }

    Graph solve() override;

private:
    void initializeDP();
    void backtrack(int treeNode, unsigned subset, uint64_t partition);

    void solveForNode(unsigned nodeId);
    void solveForSubset(unsigned nodeId, unsigned subset);
    void solveForPartition(TreeDecomposition::Node &node,
                           int nodeId, unsigned subset, uint64_t partition);

    unsigned getFromCache(int nodeId, unsigned subset, uint64_t partition);

    unsigned resolveIntroNode(TreeDecomposition::Node &node,
                              int treeNode, unsigned subset, uint64_t partition);
    unsigned resolveForgetNode(TreeDecomposition::Node &node,
                               int treeNode, unsigned int subset, uint64_t partition);
    unsigned resolveJoinNode(TreeDecomposition::Node &node,
                             int treeNode, unsigned int subset, uint64_t partition);
    unsigned resolveEdgeNode(TreeDecomposition::Node &node,
                             int treeNode, unsigned int subset, uint64_t partition);
    unsigned resolveLeafNode(unsigned int subset);

    std::vector<std::vector<std::unordered_map<uint64_t, unsigned>>> dpCache;

    struct backtrackEntry {
        int nodeId;
        unsigned subset;
        uint64_t partition;
    };

    std::vector<std::vector<std::unordered_map<uint64_t, backtrackEntry>>>
            dpBacktrack, joinBacktrack;
    std::vector<std::pair<int, int>> resultEdges;
    int globalTerminal;
    unsigned INFTY;

    clock_t introTime, forgetTime, joinTime, edgeTime, leafTime;
};


#endif //PACE2018_TABLE_DP_SOLVER_H
