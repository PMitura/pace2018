#include "table_dp_solver.h"

Graph TableDPSolver::solve() {
    initializeDP();
    globalTerminal = graph.getTerminals()[0];

    for (unsigned i = decomposition.getNodeCount(); i > 0; i--) {
        solveForNode(i - 1);
    }
    unsigned result = getFromCache(0, 1, 0);

    // TODO: non-temporary output
    std::cout << "VALUE " << result << std::endl;
    backtrack(0, 1, 0);
    for (auto edge : resultEdges) {
        std::cout << edge.first + 1 << " " << edge.second + 1 << std::endl;
    }

    /*
    std::cout << "INTRO time  " << (double)introTime / CLOCKS_PER_SEC  << "s" << std::endl;
    std::cout << "FORGET time " << (double)forgetTime / CLOCKS_PER_SEC << "s" << std::endl;
    std::cout << "JOIN time   " << (double)joinTime / CLOCKS_PER_SEC   << "s" << std::endl;
    std::cout << "EDGE time   " << (double)edgeTime / CLOCKS_PER_SEC   << "s" << std::endl;
    std::cout << "LEAF time   " << (double)leafTime / CLOCKS_PER_SEC   << "s" << std::endl;
     */

    return Graph();
}

void TableDPSolver::initializeDP() {
    unsigned treeNodes = decomposition.getNodeCount();
    dpCache.resize(treeNodes);
    dpBacktrack.resize(treeNodes);
    joinBacktrack.resize(treeNodes);
    for (unsigned i = 0; i < treeNodes; ++i) {
        auto bagSize = decomposition.getBagOf(i).size();
        // 64b variable insufficient for partitions
        if (bagSize > 16) {
            exit(1);
        }

        dpCache[i].resize(1u << bagSize);
        dpBacktrack[i].resize(1u << bagSize);
        joinBacktrack[i].resize(1u << bagSize);
    }
    resultEdges.clear();
}

void TableDPSolver::backtrack(int treeNode, unsigned subset, uint64_t partition) {
    TreeDecomposition::Node node = decomposition.getNodeAt(treeNode);
    // printDPState(node, treeNode, subset, partition);

    if (node.type == TreeDecomposition::LEAF) {
        return;
    }
    backtrackEntry next = dpBacktrack[treeNode][subset][partition], join = {-1, 0, 0};
    if (node.type == TreeDecomposition::JOIN) {
        join = joinBacktrack[treeNode][subset][partition];
    }
    switch (node.type) {
        case TreeDecomposition::INTRO:
        case TreeDecomposition::FORGET:
            backtrack(next.nodeId, next.subset, next.partition);
            break;

        case TreeDecomposition::JOIN:
            backtrack(next.nodeId, next.subset, next.partition);
            backtrack(join.nodeId, join.subset, join.partition);
            break;

        case TreeDecomposition::INTRO_EDGE:
            if (next.partition != partition) {
                resultEdges.push_back(node.associatedEdge);
            }
            backtrack(next.nodeId, next.subset, next.partition);
            break;

        case TreeDecomposition::LEAF:
            break;

        default:
            std::cerr << "Error, decomposition not nice!" << std::endl;
            exit(1);
    }
}

void TableDPSolver::solveForNode(unsigned nodeId) {
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

void TableDPSolver::solveForSubset(unsigned nodeId, unsigned subset) {
    TreeDecomposition::Node node = decomposition.getNodeAt(nodeId);

    // generate all feasible partitions
    uint64_t basePartition = 0;
    Partitioner partitioner(basePartition, subset, (unsigned)node.bag.size());
    partitioner.compute();
    const std::vector<uint64_t>& partitions = partitioner.getResult();

    for (auto part : partitions) {
        solveForPartition(node, nodeId, subset, part);
    }
}

void TableDPSolver::solveForPartition(TreeDecomposition::Node &node,
                                       int nodeId, unsigned subset, uint64_t partition) {
    unsigned result;
//    clock_t startClock = clock();
    switch (node.type) {
        case TreeDecomposition::INTRO:
            result = resolveIntroNode(node, nodeId, subset, partition);
//            introTime += (clock() - startClock);
            break;
        case TreeDecomposition::FORGET:
            result = resolveForgetNode(node, nodeId, subset, partition);
//            forgetTime += (clock() - startClock);
            break;
        case TreeDecomposition::JOIN:
            result = resolveJoinNode(node, nodeId, subset, partition);
//            joinTime += (clock() - startClock);
            break;
        case TreeDecomposition::INTRO_EDGE:
            result = resolveEdgeNode(node, nodeId, subset, partition);
//            edgeTime += (clock() - startClock);
            break;
        case TreeDecomposition::LEAF:
            result = resolveLeafNode(subset);
//            leafTime += (clock() - startClock);
            break;
        default:
            std::cerr << "Error, decomposition not nice!" << std::endl;
            exit(1);
    }
    dpCache[nodeId][subset][partition] = result;
}

unsigned TableDPSolver::getFromCache(int nodeId, unsigned subset, uint64_t partition) {
    if (dpCache[nodeId][subset].count(partition) != 0) {
        return dpCache[nodeId][subset][partition];
    }
    return INFTY;
}

unsigned TableDPSolver::resolveIntroNode(TreeDecomposition::Node &node,
                                 int treeNode, unsigned subset, uint64_t partition) {
    // get child id
    int child = 0;
    for (auto adj : node.adjacent) {
        if (adj < treeNode) {
            continue;
        }
        child = adj;
    }

    // process introduced node
    int introduced = node.associatedNode;
    unsigned idOfIntro = 0;
    while (node.bag[idOfIntro] != introduced) {
        idOfIntro++;
    }

    // check if we are introducing the global terminal
    if (introduced == globalTerminal) {
        dpBacktrack[treeNode][subset][partition] = {child, subset, partition};
        return getFromCache(child, subset, partition);
    }

    std::vector<char> vParts = partitionToVec((int)node.bag.size(), partition);
    // if the element is in used subset
    if ((subset & (1u << idOfIntro)) != 0) {
        // has to be alone in its partition
        for (unsigned i = 0; i < vParts.size(); ++i) {
            if (i != idOfIntro && vParts[i] == vParts[idOfIntro]) {
                return INFTY;
            }
        }
    }

    // prepare new partitions
    unsigned newMask = maskWithoutElement(subset, idOfIntro, (int)node.bag.size());
    uint64_t newPartition = partitionWithoutElement(vParts, idOfIntro, newMask);

    // get the solution from the child
    unsigned result = getFromCache(child, newMask, newPartition);
    dpBacktrack[treeNode][subset][partition] = {child, newMask, newPartition};
    return result;
}

unsigned TableDPSolver::resolveForgetNode(TreeDecomposition::Node &node,
                                           int treeNode, unsigned int subset, uint64_t partition) {
    // get the singular child
    int child = 0;
    for (auto adj : node.adjacent) {
        if (adj < treeNode) {
            continue;
        }
        child = adj;
    }
    const TreeDecomposition::Node &childNode = decomposition.getNodeAt(child);

    // get id of the forgotten node in child
    int forgotten = node.associatedNode, forgottenId = 0;
    if (forgotten == globalTerminal) {
        dpBacktrack[treeNode][subset][partition] = {child, subset, partition};
        return getFromCache(child, subset, partition);
    }
    while (childNode.bag[forgottenId] != forgotten) {
        forgottenId++;
    }

    unsigned newMask, bestMask = 0;
    uint64_t newPartition = 0, bestPartition = 0;
    unsigned result = INFTY;
    std::vector<char> vPartition = partitionToVec((int)node.bag.size(), partition),
            newVPartition;

    // case, where we didn't use the forgotten node
    if (!graph.isTerm(forgotten)) {
        newMask = maskWithElement(subset, (unsigned)forgottenId, 0, (int) node.bag.size());
        newVPartition = vPartition;
        newVPartition.insert(newVPartition.begin() + forgottenId, 0);
        newPartition = vecToPartition(newVPartition, newMask);
        result = getFromCache(child, newMask, newPartition);

        // keep track of the best solution for backtrack
        bestMask = newMask;
        bestPartition = newPartition;
    }

    // case, where we used the forgotten node
    newMask = maskWithElement(subset, (unsigned)forgottenId, 1, (int)node.bag.size());
    int maxPartitionId = *std::max_element(vPartition.begin(), vPartition.end());
    for (char part = 0; part < maxPartitionId + 1; part++) {
        newVPartition = vPartition;
        newVPartition.insert(newVPartition.begin() + forgottenId, part);
        newPartition = vecToPartition(newVPartition, newMask);

        unsigned candidate = getFromCache(child, newMask, newPartition);
        if (result > candidate) {
            result = candidate;
            bestMask = newMask;
            bestPartition = newPartition;
        }
    }

    dpBacktrack[treeNode][subset][partition] = {child, bestMask, bestPartition};
    return result;
}

unsigned TableDPSolver::resolveJoinNode(TreeDecomposition::Node &node,
                                         int treeNode, unsigned int subset, uint64_t partition) {
    // get children IDs
    int children[2], childPtr = 0;
    for (auto adj : node.adjacent) {
        if (adj < treeNode) {
            continue;
        }
        children[childPtr++] = adj;
    }

    // find possible partitions
    auto bagSize = node.bag.size();
    Partitioner partitioner(partition, subset, (int)bagSize);
    partitioner.compute();
    const std::vector<uint64_t> &subpartitions = partitioner.getResult();

    struct partitionResult {
        unsigned result;
        int compCount;
        uint64_t assocPartition;
    };

    // precompute component counts
    std::vector<int> compCounts;
    std::vector<partitionResult> results1, results2;
    std::vector<unsigned> compResults1, compResults2;

    // prefetch results
    for (auto i : subpartitions) {
        int compCount = maxComponentIn(i, (unsigned)bagSize) + 1;
        results1.push_back({getFromCache(children[0], subset, i), compCount, i});
        results2.push_back({getFromCache(children[1], subset, i), compCount, i});
    }

    // sort partitions by result
    auto partitionComp = [](const partitionResult& a, const partitionResult& b) {
        return a.result < b.result;
    };
    std::sort(results1.begin(), results1.end(), partitionComp);
    std::sort(results2.begin(), results2.end(), partitionComp);

    // precompute global values
    int partCompCount = maxComponentIn(partition, (unsigned)bagSize) + 1;
    int activeNodes = __builtin_popcount(subset);

    // compute result for all partition pairs
    /*
    unsigned result = getFromCache(children[0], subset, partition) +
                      getFromCache(children[1], subset, partition);
    uint64_t bestP1 = partition, bestP2 = partition;
     */
    unsigned result = INFTY;
    uint64_t bestP1 = 0, bestP2 = 0;
    UnionFindMerger merger(partition, (unsigned)bagSize, subset);
    for (auto& p1 : results1) {
        if (p1.result >= result) {
            break;
        }
        for (auto& p2 : results2) {
            unsigned candidate = p1.result + p2.result;
            if (candidate >= result) {
                break;
            }
            if (activeNodes != p1.compCount + p2.compCount - partCompCount) {
                continue;
            }
            if (merger.merge(p1.assocPartition, p2.assocPartition) != partition) {
                continue;
            }
            result = candidate;
            bestP1 = p1.assocPartition;
            bestP2 = p2.assocPartition;
            break;
        }
    }

    // write backtrack info about both branches
    dpBacktrack[treeNode][subset][partition] = {children[0], subset, bestP1};
    joinBacktrack[treeNode][subset][partition] = {children[1], subset, bestP2};
    return result;
}

unsigned TableDPSolver::resolveEdgeNode(TreeDecomposition::Node &node,
                                         int treeNode, unsigned int subset, uint64_t partition) {
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

    // find the singular child
    int child = 0;
    for (auto adj : node.adjacent) {
        if (adj < treeNode) {
            continue;
        }
        child = adj;
    }

    // if one of them is not in the selected subset
    if (!isInSubset(end1id, subset) || !isInSubset(end2id, subset)) {
        dpBacktrack[treeNode][subset][partition] = {child, subset, partition};
        return getFromCache(child, subset, partition);
    }

    // if they are in separate partitions
    if (getComponentAt(partition, end1id) != getComponentAt(partition, end2id)) {
        dpBacktrack[treeNode][subset][partition] = {child, subset, partition};
        return getFromCache(child, subset, partition);
    }

    // case when the edge is unused
    unsigned result = getFromCache(child, subset, partition);
    uint64_t bestPartition = partition;

    // setup variables for the cases where the edge is used
    std::vector<char> vPartition = partitionToVec((int)node.bag.size(), partition);
    int maxPartitionId = *std::max_element(vPartition.begin(), vPartition.end());
    int edgeWeight = graph.getAdjacentOf(intro1).at(intro2);

    // iterate over all possible partitions, where edge ends are disconnected

    std::vector<int> edgePartitionIds;
    for (unsigned i = 0; i < node.bag.size(); i++) {
        if (vPartition[i] == vPartition[end1id] && isInSubset(i, subset)) {
            edgePartitionIds.push_back(i);
        }
    }
    auto edgePartitionSize = edgePartitionIds.size();

    for (unsigned option = 0; option < (1u << edgePartitionSize); option++) {
        std::vector<char> newVPartition = vPartition;
        unsigned ctr = 0;
        for (auto id : edgePartitionIds) {
            if ((option & (1u << ctr++)) != 0) {
                // set new separate partition
                newVPartition[id] = (char)(maxPartitionId + 1);
            }
        }
        if (newVPartition[end1id] == newVPartition[end2id]) {
            continue;
        }
        uint64_t newPartition = vecToPartition(newVPartition, subset);

        unsigned candidate = getFromCache(child, subset, newPartition);
        if (result > candidate + edgeWeight) {
            result = candidate + edgeWeight;
            bestPartition = newPartition;
        }
    }

    dpBacktrack[treeNode][subset][partition] = {child, subset, bestPartition};
    return result;
}

unsigned TableDPSolver::resolveLeafNode(unsigned int subset) {
    if (subset == 1) {
        return 0;
    }
    return INFTY;
}
