#include "reduce_dp_solver.h"

Graph ReduceDPSolver::solve() {
    initializeDP();
    markDeletableNodes();

    for (unsigned i = decomposition.getNodeCount(); i > 0; i--) {
        solveForNode(i - 1);
        for (auto child : decomposition.getAdjacentTo(i-1)) {
            if (deletable[child]) {
                eraseNodeBacktrack((unsigned)child);
            }
        }
    }

    // TODO: non-temporary output
    FullBacktrackEntry startPoint = findResult();
    unsigned result = dpCache[startPoint.nodeId][startPoint.subset][startPoint.partition];

    std::cout << "VALUE " << result + graph.getPreselectedWeight() << std::endl;
    backtrack(startPoint.nodeId, startPoint.subset, startPoint.partition);
    for (auto edge : resultEdges) {
        std::cout << edge.first + 1 << " " << edge.second + 1 << std::endl;
    }
    for (auto edge : graph.getPreselectedEdges()) {
        std::cout << edge.first + 1 << " " << edge.second + 1 << std::endl;
    }

    std::cout << "PARTITIONING time    " << (double)partTime / CLOCKS_PER_SEC    << "s" << std::endl;
    std::cout << "CUT MATRIX GEN time  " << (double)matrixTime / CLOCKS_PER_SEC  << "s" << std::endl;
    std::cout << "CUT MATRIX ELIM time " << (double)elimTime / CLOCKS_PER_SEC  << "s" << std::endl;
    std::cout << "REDUCE OVERHEAD time " << (double)overheadTime / CLOCKS_PER_SEC  << "s" << std::endl;

    return Graph();
}

void ReduceDPSolver::initializeDP() {
    unsigned treeNodes = decomposition.getNodeCount();
    dpCache.resize(treeNodes);
    dpBacktrack.resize(treeNodes);
    for (unsigned i = 0; i < treeNodes; ++i) {
        auto bagSize = decomposition.getBagOf(i).size();
        // 64b variable insufficient for partitions
        if (bagSize > 16) {
            std::cerr << "Error: bags too large!" << std::endl;
            exit(1);
        }

        dpCache[i].resize(1u << bagSize);
        dpBacktrack[i].resize(1u << bagSize);
    }
    resultEdges.clear();
}

void ReduceDPSolver::markDeletableNodes() {
    int nodeId = 1;
    deletable.resize(decomposition.getNodeCount(), true);
    bool stopAtNext = false;
    if (graph.isTerm(decomposition.getNodeAt(0).associatedNode)) {
        stopAtNext = true;
    }
    while (true) {
        TreeDecomposition::Node node = decomposition.getNodeAt(nodeId);
        deletable[nodeId] = false;

        if (stopAtNext) {
            break;
        }

        if (node.type == TreeDecomposition::JOIN) {
            bool branchTerm1 = branchContainsTerminal(node.adjacent[0]),
                    branchTerm2 = branchContainsTerminal(node.adjacent[1]);

            if (branchTerm1 && branchTerm2) {
                break;
            }
            if (!branchTerm1 && !branchTerm2) {
                std::cerr << "No terminal found in the tree" << std::endl;
            }
            if (branchTerm1) {
                nodeId = node.adjacent[0];
                continue;
            }
            if (branchTerm2) {
                nodeId = node.adjacent[1];
                continue;
            }
        }
        if (node.type == TreeDecomposition::FORGET && graph.isTerm(node.associatedNode)) {
            stopAtNext = true;
        }

        nodeId++;
    }
}

ReduceDPSolver::FullBacktrackEntry ReduceDPSolver::findResult() {
    unsigned bestResult = UINT_MAX;
    FullBacktrackEntry bestEntry = {0, 0, 0};
    for (unsigned nodeId = 1; nodeId < decomposition.getNodeCount(); nodeId++) {
        if (deletable[nodeId]) {
            continue;
        }

        TreeDecomposition::Node node = decomposition.getNodeAt(nodeId);
        unsigned termMask = 0, termCount = 0, varCount = 0;
        for (unsigned i = 0; i < node.bag.size(); i++) {
            if (graph.isTerm(node.bag[i])) {
                termMask |= 1u << i;
                termCount++;
            } else {
                varCount++;
            }
        }

        for (unsigned varSubset = 0; varSubset < (1u << varCount); varSubset++) {
            // scatter variables to the full subset
            uint16_t subset = 0;
            unsigned varIdx = 0;
            for (unsigned i = 0; i < varCount + termCount; i++) {
                if ((termMask & (1u << i)) != 0) {
                    subset |= (1u << i);
                } else {
                    if ((varSubset & (1u << varIdx)) != 0) {
                        subset |= (1u << i);
                    }
                    varIdx++;
                }
            }

            unsigned candidate = UINT_MAX;
            if (dpCache[nodeId][subset].count(0) != 0) {
                candidate = dpCache[nodeId][subset][0];
            }
            if (candidate < bestResult) {
                bestResult = candidate;
                bestEntry = {nodeId, subset, 0};
            }
        }
    }

    return bestEntry;
}

bool ReduceDPSolver::branchContainsTerminal(int nodeId) {
    TreeDecomposition::Node node = decomposition.getNodeAt(nodeId);

    for (auto item : node.bag) {
        if (graph.isTerm(item)) {
            return true;
        }
    }

    if (node.type == TreeDecomposition::LEAF) {
        return false;
    }

    if (node.type == TreeDecomposition::JOIN) {
        return branchContainsTerminal(node.adjacent[0]) || branchContainsTerminal(node.adjacent[1]);
    }
    return branchContainsTerminal(node.adjacent[0]);
}


void ReduceDPSolver::backtrack(int treeNode, unsigned subset, uint64_t partition) {
    EdgeBacktrack edges = dpBacktrack[treeNode][subset][partition];

    for (int edgeId = 0; edgeId < graph.getEdgeCount(); edgeId++) {
        if (edges.get((unsigned)edgeId)) {
            resultEdges.push_back(graph.edgeWithId(edgeId));
        }
    }
}

void ReduceDPSolver::solveForNode(unsigned nodeId) {
    TreeDecomposition::Node node = decomposition.getNodeAt(nodeId);

    // find all subsets, terminals always stay on
    unsigned termMask = 0, termCount = 0, varCount = 0;
    for (unsigned i = 0; i < node.bag.size(); i++) {
        if (graph.isTerm(node.bag[i])) {
            termMask |= 1u << i;
            termCount++;
        } else {
            varCount++;
        }
    }

    // iterate over subsets of variable nodes
    for (unsigned varSubset = 0; varSubset < (1u << varCount); varSubset++) {
        // scatter variables to the full subset
        unsigned subset = 0, varIdx = 0;
        for (unsigned i = 0; i < varCount + termCount; i++) {
            if ((termMask & (1u << i)) != 0) {
                subset |= (1u << i);
            } else {
                if ((varSubset & (1u << varIdx)) != 0) {
                    subset |= (1u << i);
                }
                varIdx++;
            }
        }

        solveForSubset(nodeId, subset);
    }
}

void ReduceDPSolver::eraseNodeBacktrack(unsigned nodeId) {
    TreeDecomposition::Node node = decomposition.getNodeAt(nodeId);

    // find all subsets, terminals always stay on
    unsigned termMask = 0, termCount = 0, varCount = 0;
    for (unsigned i = 0; i < node.bag.size(); i++) {
        if (graph.isTerm(node.bag[i])) {
            termMask |= 1u << i;
            termCount++;
        } else {
            varCount++;
        }
    }

    // iterate over subsets of variable nodes
    for (unsigned varSubset = 0; varSubset < (1u << varCount); varSubset++) {
        // scatter variables to the full subset
        unsigned subset = 0, varIdx = 0;
        for (unsigned i = 0; i < varCount + termCount; i++) {
            if ((termMask & (1u << i)) != 0) {
                subset |= (1u << i);
            } else {
                if ((varSubset & (1u << varIdx)) != 0) {
                    subset |= (1u << i);
                }
                varIdx++;
            }
        }

        dpCache[nodeId][subset].clear();
        dpBacktrack[nodeId][subset].clear();
    }
}

void ReduceDPSolver::solveForSubset(unsigned nodeId, unsigned subset) {
    TreeDecomposition::Node node = decomposition.getNodeAt(nodeId);

    clock_t startTime = clock();
    std::vector<uint64_t> partitions = generateParts(nodeId, subset);
    partTime += clock() - startTime;

    // reduce the number of partitions
    if (subset != 0) {
        reduce(nodeId, subset);
    }
}

void ReduceDPSolver::reduce(unsigned nodeId, unsigned subset) {
    clock_t startTime = clock();
    TreeDecomposition::Node node = decomposition.getNodeAt(nodeId);

    std::vector<uint64_t> partitions;

    for (auto entry : dpCache[nodeId][subset]) {
        partitions.push_back(entry.first);
    }
    overheadTime += clock() - startTime;

    std::vector<uint64_t> reducedPartitions;
    if ((partitions.size() << 1u) > (1u << (unsigned)(__builtin_popcount(subset)))) {
        std::sort(partitions.begin(), partitions.end(), [&](const uint64_t& a, const uint64_t& b) {
            return dpCache[nodeId][subset][a] < dpCache[nodeId][subset][b];
        });

        startTime = clock();
        CutMatrix cutMatrix;
        cutMatrix.generate(partitions, subset, (unsigned) node.bag.size());
        matrixTime += clock() - startTime;

        startTime = clock();
        cutMatrix.eliminate();
        reducedPartitions = cutMatrix.getPartitions();
        elimTime += clock() - startTime;
    } else {
        reducedPartitions = partitions;
    }

    startTime = clock();
    std::unordered_map<uint64_t, unsigned> newPart;
    for (auto part : reducedPartitions) {
        newPart[part] = dpCache[nodeId][subset][part];
    }
    dpCache[nodeId][subset] = newPart;
    overheadTime += clock() - startTime;
}

std::vector<uint64_t> ReduceDPSolver::generateIntroParts(int nodeId, unsigned subset, uint64_t sourcePart,
                                                         unsigned childSubset) {
    TreeDecomposition::Node node = decomposition.getNodeAt(nodeId);
    int child = node.adjacent[0];

    // get the id of the introduced node
    int introduced = node.associatedNode;
    unsigned candidate = dpCache[child][childSubset][sourcePart];
    unsigned introducedId = 0;
    while (node.bag[introducedId] != introduced) {
        introducedId++;
    }

    // find new partition ID
    char newPartitionId = 0;
    std::vector<char> vPartition = partitionToVec((unsigned)node.bag.size() - 1, sourcePart);
    if (isInSubset(introducedId, subset)) {
        unsigned sourceSubset = maskWithoutElement(subset, introducedId, (unsigned)node.bag.size());
        char maxPartitionId = 0;
        for (unsigned i = 0; i < vPartition.size(); i++) {
            if (vPartition[i] > maxPartitionId && isInSubset(i, sourceSubset)) {
                maxPartitionId = vPartition[i];
            }
        }
        newPartitionId = maxPartitionId + (char)1;
    }

    // assign the new partition to the introduced node
    vPartition.insert(vPartition.begin() + introducedId, newPartitionId);
    uint64_t parentPart = vecToPartition(vPartition, subset);
    if (dpCache[nodeId][subset].count(parentPart) == 0
        || candidate < dpCache[nodeId][subset][parentPart]) {
        dpCache[nodeId][subset][parentPart] = candidate;
        dpBacktrack[nodeId][subset][parentPart] = dpBacktrack[child][childSubset][sourcePart];
    }
    return {parentPart};
}

std::vector<uint64_t> ReduceDPSolver::generateForgetParts(int nodeId, unsigned subset, uint64_t sourcePart,
                                                          unsigned childSubset) {
    TreeDecomposition::Node node = decomposition.getNodeAt(nodeId);

    // get the singular child
    int child = node.adjacent[0];
    const TreeDecomposition::Node &childNode = decomposition.getNodeAt(child);

    // get id of the forgotten node in child
    int forgotten = node.associatedNode;
    unsigned forgottenId = 0;
    unsigned candidate = dpCache[child][childSubset][sourcePart];
    while (childNode.bag[forgottenId] != forgotten) {
        forgottenId++;
    }

    // if forgotten node is in a separate partition, there must be a better solution
    std::vector<char> vChildPartition = partitionToVec((unsigned)childNode.bag.size(), sourcePart);
    if (isInSubset(forgottenId, childSubset)) {
        bool foundAdj = false;
        for (unsigned i = 0; i < vChildPartition.size(); i++) {
            if (isInSubset(i, childSubset) && i != forgottenId &&
                vChildPartition[i] == vChildPartition[forgottenId]) {
                foundAdj = true;
                break;
            }
        }
        if (!foundAdj) {
            return {};
        }
    }

    uint64_t parentPartition = partitionWithoutElement(vChildPartition, forgottenId, subset);

    // forward the results
    if (dpCache[nodeId][subset].count(parentPartition) == 0
        || candidate < dpCache[nodeId][subset][parentPartition]) {
        dpCache[nodeId][subset][parentPartition] = candidate;
        dpBacktrack[nodeId][subset][parentPartition] = dpBacktrack[child][childSubset][sourcePart];
    }

    return {parentPartition};
}

std::vector<uint64_t> ReduceDPSolver::generateJoinParts(int nodeId, unsigned subset,
                                                        const std::vector<uint64_t>& sourceParts1,
                                                        const std::vector<uint64_t>& sourceParts2) {
    TreeDecomposition::Node node = decomposition.getNodeAt(nodeId);

    int children[2] = {-1, -1}, childPtr = 0;
    for (auto adj : node.adjacent) {
        if (adj < nodeId) {
            continue;
        }
        children[childPtr++] = adj;
    }

    // try to merge all partition pairs
    std::unordered_set<uint64_t> partitions;
    UnionFindMerger merger((unsigned)node.bag.size(), subset);
    for (auto p1 : sourceParts1) {
        for (auto p2 : sourceParts2) {
            uint64_t merged = merger.merge(p1, p2);

            // precompute results
            unsigned candidate = dpCache[children[0]][subset][p1]
                               + dpCache[children[1]][subset][p2];

            if (dpCache[nodeId][subset].count(merged) == 0
                || candidate < dpCache[nodeId][subset][merged]) {
                dpCache[nodeId][subset][merged] = candidate;

                // compute backtrack info
                EdgeBacktrack edges1 = dpBacktrack[children[0]][subset][p1],
                              edges2 = dpBacktrack[children[1]][subset][p2];
                edges1.mergeWith(edges2);
                dpBacktrack[nodeId][subset][merged] = edges1;
            }

            partitions.insert(merged);
        }
    }

    std::vector<uint64_t> vPartitions(partitions.begin(), partitions.end());
    return vPartitions;
}

std::vector<uint64_t> ReduceDPSolver::generateEdgeParts(int nodeId, unsigned subset, uint64_t sourcePart) {
    TreeDecomposition::Node node = decomposition.getNodeAt(nodeId);
    // get the singular child
    int child = node.adjacent[0];
    std::vector<uint64_t> partitions;

    // case where we don't use the edge, forward the result to the cache
    partitions.push_back(sourcePart);
    unsigned candidate = dpCache[child][subset][sourcePart];
    if (dpCache[nodeId][subset].count(sourcePart) == 0
        || dpCache[nodeId][subset][sourcePart] > candidate) {
        dpCache[nodeId][subset][sourcePart] = candidate;
        dpBacktrack[nodeId][subset][sourcePart] = dpBacktrack[child][subset][sourcePart];
    }

    std::vector<char> vPartition = partitionToVec((unsigned)node.bag.size(), sourcePart);

    // get both endpoints of the new edge
    int intro1 = node.associatedEdge.first,
        intro2 = node.associatedEdge.second;

    // get both edge endpoint ids
    unsigned end1id = 0, end2id = 0;
    while (node.bag[end1id] != intro1) {
        end1id++;
    }
    while (node.bag[end2id] != intro2) {
        end2id++;
    }

    if (!isInSubset(end1id, subset) || !isInSubset(end2id, subset)) {
        return partitions;
    }

    // add weight of the edge to the candidate solution (edge is used)
    candidate += graph.getAdjacentOf(intro1).at(intro2);

    // if edge is used, merge the parts above
    if (vPartition[end1id] != vPartition[end2id]) {
        char partToReplace = vPartition[end2id], replaceBy = vPartition[end1id];
        for (auto& comp : vPartition) {
            if (comp == partToReplace) {
                comp = replaceBy;
            }
        }
        uint64_t newPart = vecToPartition(vPartition, subset);
        partitions.push_back(newPart);

        // forward the result to the table
        if (dpCache[nodeId][subset].count(newPart) == 0
            || dpCache[nodeId][subset][newPart] > candidate) {
            dpCache[nodeId][subset][newPart] = candidate;
            // add edge to the backtrack table
            EdgeBacktrack btEdge = dpBacktrack[child][subset][sourcePart];
            btEdge.turnOn((unsigned)graph.idOfEdge(std::minmax(intro1, intro2)));
            dpBacktrack[nodeId][subset][newPart] = btEdge;
        }
    }

    return partitions;
}

std::vector<uint64_t> ReduceDPSolver::generateParts(int nodeId, unsigned subset) {
    TreeDecomposition::Node node = decomposition.getNodeAt(nodeId);
    int children[2] = {-1, -1}, childPtr = 0;
    for (auto adj : node.adjacent) {
        if (adj < nodeId) {
            continue;
        }
        children[childPtr++] = adj;
    }

    if (node.type == TreeDecomposition::JOIN) {
        std::vector<uint64_t> source1, source2;
        for (auto i : dpCache[children[0]][subset]) {
            source1.push_back(i.first);
        }
        for (auto i : dpCache[children[1]][subset]) {
            source2.push_back(i.first);
        }
        return generateJoinParts(nodeId, subset, source1, source2);
    }


    if (node.type == TreeDecomposition::LEAF) {
        dpCache[nodeId][subset][0] = 0;
        dpBacktrack[nodeId][subset][0] = EdgeBacktrack((unsigned)graph.getEdgeCount());
        return {0};
    }

    std::unordered_set<uint64_t> setResult;

    if (node.type == TreeDecomposition::INTRO) {
        int introduced = node.associatedNode;
        unsigned introducedId = 0;
        while (node.bag[introducedId] != introduced) {
            introducedId++;
        }
        unsigned childSubset = maskWithoutElement(subset, introducedId, (unsigned)node.bag.size());
        for (auto i : dpCache[children[0]][childSubset]) {
            std::vector<uint64_t> generatedByPart = generateIntroParts(nodeId, subset, i.first, childSubset);
            for (auto part : generatedByPart) {
                setResult.insert(part);
            }
        }
    }

    if (node.type == TreeDecomposition::FORGET) {
        const TreeDecomposition::Node &childNode = decomposition.getNodeAt(children[0]);

        // get id of the forgotten node in child
        int forgotten = node.associatedNode;
        unsigned childSubset1, childSubset2, forgottenId = 0;
        while (childNode.bag[forgottenId] != forgotten) {
            forgottenId++;
        }
        childSubset1 = maskWithElement(subset, forgottenId, 0, (unsigned)node.bag.size());
        childSubset2 = maskWithElement(subset, forgottenId, 1, (unsigned)node.bag.size());

        // forgotten node wasn't used
        if (!graph.isTerm(forgotten)) {
            for (auto i : dpCache[children[0]][childSubset1]) {
                std::vector<uint64_t> generatedByPart = generateForgetParts(nodeId, subset, i.first, childSubset1);
                for (auto part : generatedByPart) {
                    setResult.insert(part);
                }
            }
        }

        // forgotten node was used
        for (auto i : dpCache[children[0]][childSubset2]) {
            std::vector<uint64_t> generatedByPart = generateForgetParts(nodeId, subset, i.first, childSubset2);
            for (auto part : generatedByPart) {
                setResult.insert(part);
            }
        }
    }

    if (node.type == TreeDecomposition::INTRO_EDGE) {
        for (auto i : dpCache[children[0]][subset]) {
            std::vector<uint64_t> generatedByPart = generateEdgeParts(nodeId, subset, i.first);
            for (auto part : generatedByPart) {
                setResult.insert(part);
            }
        }
    }

    std::vector<uint64_t> result(setResult.begin(), setResult.end());
    return result;
}
