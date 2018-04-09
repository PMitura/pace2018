#ifndef PACE2018_REDUCE_DP_SOLVER_H
#define PACE2018_REDUCE_DP_SOLVER_H

#include <climits>
#include <ctime>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "solvers/solver.h"
#include "structures/cut_matrix.h"
#include "utility/partitioner.h"
#include "utility/partition_mergers.h"

class ReduceDPSolver : public Solver {
public:
    ReduceDPSolver(const Graph &inputGraph, const TreeDecomposition &niceDecomposition)
            : Solver(inputGraph, niceDecomposition),
              globalTerminal(-1),
              matrixTime(0), elimTime(0), partTime(0), overheadTime(0) {
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

    void reduce(unsigned nodeId, unsigned subset);

    std::vector<uint64_t> generateParts(int nodeId, unsigned subset);

    std::vector<uint64_t> generateIntroParts(int nodeId, unsigned subset, uint64_t sourcePart,
                                             unsigned childSubset);
    std::vector<uint64_t> generateForgetParts(int nodeId, unsigned subset, uint64_t sourcePart,
                                              unsigned childSubset);
    std::vector<uint64_t> generateJoinParts(int nodeId, unsigned subset,
                                            const std::vector<uint64_t>& sourceParts1,
                                            const std::vector<uint64_t>& sourceParts2);
    std::vector<uint64_t> generateEdgeParts(int nodeId, unsigned subset, uint64_t sourcePart);

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

    clock_t matrixTime, elimTime, partTime, overheadTime;
};


#endif //PACE2018_REDUCE_DP_SOLVER_H
