#include <utility/partitioner.h>
#include "base_dp_solver.h"

Graph BaseDPSolver::solve() {
    initializeDP();
    globalTerminal = graph.getTerminals()[0];

    unsigned result = solveInstance(0, 1, 0);

    // TODO: non-temporary output
    std::cout << "VALUE " << result << std::endl;
    // std::cout << std::endl << "BACKTRACK INFO" << std::endl;
    backtrack(0, 1, 0);
    for (auto edge : resultEdges) {
        std::cout << edge.first + 1 << " " << edge.second + 1 << std::endl;
    }

    return Graph();
}

unsigned BaseDPSolver::solveInstance(int treeNode, unsigned int subset, uint64_t partition) {
    if (dpCache[treeNode][subset].count(partition) != 0) {
        return dpCache[treeNode][subset][partition];
    }

    TreeDecomposition::Node node = decomposition.getNodeAt(treeNode);
    // printDPState(node, treeNode, subset, partition);

    unsigned result;
    switch (node.type) {
        case TreeDecomposition::INTRO:
            result = resolveIntroNode(node, treeNode, subset, partition);
            break;
        case TreeDecomposition::FORGET:
            result = resolveForgetNode(node, treeNode, subset, partition);
            break;
        case TreeDecomposition::JOIN:
            result = resolveJoinNode(node, treeNode, subset, partition);
            break;
        case TreeDecomposition::INTRO_EDGE:
            result = resolveEdgeNode(node, treeNode, subset, partition);
            break;
        case TreeDecomposition::LEAF:
            result = resolveLeafNode(subset);
            break;
        default:
            std::cerr << "Error, decomposition not nice!" << std::endl;
            exit(1);
    }

    dpCache[treeNode][subset][partition] = result;
    return result;
}

void BaseDPSolver::backtrack(int treeNode, int subset, uint64_t partition) {
    TreeDecomposition::Node node = decomposition.getNodeAt(treeNode);
    // printDPState(node, treeNode, subset, partition);

    if (node.type == TreeDecomposition::LEAF) {
        return;
    }
    std::vector<uint64_t> next = dpBacktrack[treeNode][subset][partition];
    switch (node.type) {
        case TreeDecomposition::INTRO:
        case TreeDecomposition::FORGET:
            backtrack((int)next[0], (int)next[1], next[2]);
            break;

        case TreeDecomposition::JOIN:
            backtrack((int)next[0], (int)next[1], next[2]);
            backtrack((int)next[3], (int)next[4], next[5]);
            break;

        case TreeDecomposition::INTRO_EDGE:
            if (next[2] != partition) {
                resultEdges.push_back(node.associatedEdge);
            }
            backtrack((int)next[0], (int)next[1], next[2]);
            break;

        case TreeDecomposition::LEAF:
            break;

        default:
            std::cerr << "Error, decomposition not nice!" << std::endl;
            exit(1);
    }
}

void BaseDPSolver::initializeDP() {
    unsigned treeNodes = decomposition.getNodeCount();
    dpCache.resize(treeNodes);
    dpBacktrack.resize(treeNodes);
    for (unsigned i = 0; i < treeNodes; ++i) {
        auto bagSize = decomposition.getBagOf(i).size();
        // 64b variable insufficient for partitions
        if (bagSize > 16) {
            exit(1);
        }

        dpCache[i].resize(1u << bagSize);
        dpBacktrack[i].resize(1u << bagSize);
    }
    resultEdges.clear();
}

unsigned BaseDPSolver::resolveIntroNode(TreeDecomposition::Node &node,
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
        return solveInstance(child, subset, partition);
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
    } else {
        // cannot have unselected terminal
        if (graph.isTerm(introduced)) {
            return INFTY;
        }
    }

    // prepare new partitions
    unsigned newMask = maskWithoutElement(subset, idOfIntro, (int)node.bag.size());
    uint64_t newPartition = partitionWithoutElement(vParts, idOfIntro, newMask);

    // get the solution from the child
    unsigned result = solveInstance(child, newMask, newPartition);
    dpBacktrack[treeNode][subset][partition] = {child, newMask, newPartition};
    return result;
}

unsigned BaseDPSolver::resolveForgetNode(TreeDecomposition::Node &node,
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
        return solveInstance(child, subset, partition);
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
        result = solveInstance(child, newMask, newPartition);

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

        unsigned candidate = solveInstance(child, newMask, newPartition);
        if (result > candidate) {
            result = candidate;
            bestMask = newMask;
            bestPartition = newPartition;
        }
    }

    dpBacktrack[treeNode][subset][partition] = {child, bestMask, bestPartition};
    return result;
}

unsigned BaseDPSolver::resolveJoinNode(TreeDecomposition::Node &node,
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

    // compute result for all partition pairs
    unsigned result = INFTY;
    uint64_t bestP1 = 0, bestP2 = 0;
    BinaryDFSMerger merger(partition, (unsigned)bagSize, subset);
    for (auto p1 : subpartitions) {
        for (auto p2 : subpartitions) {
            if (merger.merge(p1, p2) != partition) {
                continue;
            }
            unsigned candidate = solveInstance(children[0], subset, p1)
                               + solveInstance(children[1], subset, p2);
            if (result > candidate) {
                result = candidate;
                bestP1 = p1;
                bestP2 = p2;
            }
        }
    }

    // write backtrack info about both branches
    dpBacktrack[treeNode][subset][partition] = {children[0], subset, bestP1,
                                                children[1], subset, bestP2};
    return result;
}

unsigned BaseDPSolver::resolveEdgeNode(TreeDecomposition::Node &node,
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
        return solveInstance(child, subset, partition);
    }

    // if they are in separate partitions
    if (getComponentAt(partition, end1id) != getComponentAt(partition, end2id)) {
        dpBacktrack[treeNode][subset][partition] = {child, subset, partition};
        return solveInstance(child, subset, partition);
    }

    // case when the edge is unused
    unsigned result = solveInstance(child, subset, partition);
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

        unsigned candidate = solveInstance(child, subset, newPartition);
        if (result > candidate + edgeWeight) {
            result = candidate + edgeWeight;
            bestPartition = newPartition;
        }
    }

    dpBacktrack[treeNode][subset][partition] = {child, subset, bestPartition};
    return result;
}

unsigned BaseDPSolver::resolveLeafNode(unsigned int subset) {
    if (subset == 1) {
        return 0;
    }
    return INFTY;
}

void BaseDPSolver::printDPState(TreeDecomposition::Node &node,
                                int treeNode, unsigned int subset, uint64_t partition) {
    std::cout << "Node ID: " << treeNode << " of type ";
    switch(node.type) {
        case TreeDecomposition::INTRO:
            std::cout << "INTRO";
            break;
        case TreeDecomposition::FORGET:
            std::cout << "FORGET";
            break;
        case TreeDecomposition::JOIN:
            std::cout << "JOIN";
            break;
        case TreeDecomposition::INTRO_EDGE:
            std::cout << "INTRO_EDGE";
            break;
        case TreeDecomposition::LEAF:
            std::cout << "LEAF";
            break;
        default:
            std::cout << "!!! NOT NICE";
    }
    std::cout << "  in bag:";
    for (auto i : node.bag) {
        std::cout << " " << i;
    }
    std::cout << "  used:";
    unsigned idx = 0;
    for (auto i : node.bag) {
        if ((subset & (1u << idx)) != 0) {
            std::cout << " " << i;
        }
        idx++;
    }
    std::cout << "  comps:";
    std::vector<char> vp = partitionToVec((int)node.bag.size(), partition);
    for (idx = 0; idx < node.bag.size(); ++idx) {
        if ((subset & (1u << idx)) != 0) {
            std::cout << " " << (int)vp[idx];
        }
    }
    // std::cout << " Parts: " << std::hex << partition << std::dec;
    std::cout << std::endl;
}

