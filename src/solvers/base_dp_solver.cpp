#include <utility/partitioner.h>
#include "base_dp_solver.h"

Graph BaseDPSolver::solve() {
    initializeDP();
    globalTerminal = graph.getTerminals()[0];

    int result = solveInstance(0, 1, 0);

    // TODO: non-temporary output
    std::cout << "VALUE " << result << std::endl;

    cleanupDP();

    // TODO: backtrack
    return Graph();
}

int BaseDPSolver::solveInstance(int treeNode, int usedMask, uint64_t partition) {
    if (dpCache[treeNode][usedMask].count(partition)) {
        return dpCache[treeNode][usedMask][partition];
    }

    // TODO: remove debug log, move node initialization down
    TreeDecomposition::Node node = decomposition.getNodeAt(treeNode);
    printDPState(node, treeNode, usedMask, partition);

    int result;
    switch (node.type) {
        case TreeDecomposition::INTRO:
            result = resolveIntroNode(node, treeNode, usedMask, partition);
            break;
        case TreeDecomposition::FORGET:
            result = resolveForgetNode(node, treeNode, usedMask, partition);
            break;
        case TreeDecomposition::JOIN:
            result = resolveJoinNode(node, treeNode, usedMask, partition);
            break;
        case TreeDecomposition::INTRO_EDGE:
            result = resolveEdgeNode(node, treeNode, usedMask, partition);
            break;
        case TreeDecomposition::LEAF:
            result = resolveLeafNode(usedMask);
            break;
        default:
            std::cerr << "Error, decomposition not nice!" << std::endl;
            exit(1);
    }

    dpCache[treeNode][usedMask][partition] = result;
    return result;
}

void BaseDPSolver::initializeDP() {
    int treeNodes = decomposition.getNodeCount();
    dpCache = new std::unordered_map<uint64_t, int> * [treeNodes];
    for (int i = 0; i < treeNodes; ++i) {
        auto bagSize = decomposition.getBagOf(i).size();
        dpCache[i] = new std::unordered_map<uint64_t, int> [1 << bagSize];
    }
}

void BaseDPSolver::cleanupDP() {
    int treeNodes = decomposition.getNodeCount();
    for (int i = 0; i < treeNodes; ++i) {
        delete[] dpCache[i];
    }
    delete[] dpCache;
}

int BaseDPSolver::resolveIntroNode(TreeDecomposition::Node &node,
                                   int treeNode, int usedMask, uint64_t partition) {
    // get child id
    int child = 0;
    for (auto adj : node.adjacent) {
        if (adj < treeNode) continue;
        child = adj;
    }

    // process introduced node, check if we are not introducing the global terminal
    int introduced = node.associatedNode;
    if (introduced == globalTerminal) {
        return solveInstance(child, usedMask, partition);
    }
    int idOfIntro = 0;
    while (node.bag[idOfIntro] != introduced) idOfIntro++;

    std::vector<char> vParts = partitionToVec((int)node.bag.size(), partition);
    // if the element is in used subset
    if (usedMask & (1 << idOfIntro)) {
        // has to be alone in its partition
        for (int i = 0; i < (int)vParts.size(); ++i) {
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
    int newMask = maskWithoutElement(usedMask, idOfIntro, (int)node.bag.size());
    uint64_t newPartition = partitionWithoutElement(vParts, idOfIntro, newMask);

    // get the solution from the child
    int result = INFTY;
    for (auto adj : node.adjacent) {
        // skip parent
        if (adj < treeNode) continue;
        result = solveInstance(adj, newMask, newPartition);
    }
    return result;
}

int BaseDPSolver::resolveForgetNode(TreeDecomposition::Node &node,
                                    int treeNode, int usedMask, uint64_t partition) {
    // get the singular child
    int child = 0;
    for (auto adj : node.adjacent) {
        if (adj < treeNode) continue;
        child = adj;
    }
    const TreeDecomposition::Node &childNode = decomposition.getNodeAt(child);

    // get id of the forgotten node in child
    int forgotten = node.associatedNode, forgottenId = 0;
    if (forgotten == globalTerminal) {
        return solveInstance(child, usedMask, partition);
    }
    while (childNode.bag[forgottenId] != forgotten) forgottenId++;

    // case, where we didn't use the forgotten node
    int newMask = maskWithElement(usedMask, forgottenId, 0, (int)node.bag.size());
    std::vector<char> vPartition = partitionToVec((int)node.bag.size(), partition);
    std::vector<char> newVPartition = vPartition;
    newVPartition.insert(newVPartition.begin() + forgottenId, 0);
    int resultUnused = solveInstance(child, newMask, vecToPartition(newVPartition, newMask));

    // case, where we used the forgotten node
    newMask = maskWithElement(usedMask, forgottenId, 1, (int)node.bag.size());
    int maxPartitionId = *std::max_element(vPartition.begin(), vPartition.end());
    int resultUsed = INFTY;
    for (char part = 0; part < maxPartitionId + 1; part++) {
        newVPartition = vPartition;
        newVPartition.insert(newVPartition.begin() + forgottenId, part);

        int candidate = solveInstance(child, newMask, vecToPartition(newVPartition, newMask));
        resultUsed = std::min(resultUsed, candidate);
    }

    return std::min(resultUnused, resultUsed);
}

int BaseDPSolver::resolveJoinNode(TreeDecomposition::Node &node,
                                  int treeNode, int usedMask, uint64_t partition) {
    // get children IDs
    int children[2], childPtr = 0;
    for (auto adj : node.adjacent) {
        if (adj < treeNode) continue;
        children[childPtr++] = adj;
    }

    // try all partition pairs
    Partitioner partitioner(partition, usedMask, (int)node.bag.size());
    partitioner.compute();
    const std::vector<uint64_t> &subpartitions = partitioner.getResult();
    int result = INFTY;
    for (auto p1 : subpartitions) {
        for (auto p2 : subpartitions) {
            // TODO: this is probably needed
            // if (cyclicMerge(p1, p2) != partition) continue;
            result = std::min(result,
                  solveInstance(children[0], usedMask, p1)
                + solveInstance(children[1], usedMask, p2));
        }
    }
    return result;
}

int BaseDPSolver::resolveEdgeNode(TreeDecomposition::Node &node,
                                  int treeNode, int usedMask, uint64_t partition) {
    // get both endpoints of the new edge
    int intro1 = node.associatedEdge.first,
        intro2 = node.associatedEdge.second;

    // get both edge endpoint ids
    int end1id = 0, end2id = 0;
    while (node.bag[end1id] != intro1) end1id++;
    while (node.bag[end2id] != intro2) end2id++;

    // find the singular child
    int child = 0;
    for (auto adj : node.adjacent) {
        if (adj < treeNode) continue;
        child = adj;
    }

    // if one of them is not in the selected subset
    if (!(usedMask & (1 << end1id)) || !(usedMask & (1 << end2id))) {
        return solveInstance(child, usedMask, partition);
    }

    // if they are in separate partitions
    if (getComponentAt(partition, end1id) != getComponentAt(partition, end2id)) {
        return solveInstance(child, usedMask, partition);
    }

    // case when the edge is unused
    int resultUnused = solveInstance(child, usedMask, partition);

    // setup variables for the cases where the edge is used
    std::vector<char> vPartition = partitionToVec((int)node.bag.size(), partition);
    int maxPartitionId = *std::max_element(vPartition.begin(), vPartition.end());
    int edgeWeight = graph.getAdjacentOf(intro1).at(intro2);

    // iterate over all possible partition pairs, get minimum solution
    int resultUsed = INFTY;
    for (char part1 = 0; part1 < maxPartitionId + 2; part1++) {
        for (char part2 = 0; part2 < maxPartitionId + 2; part2++) {
            if (part1 == part2) continue;

            std::vector<char> newVPartition = vPartition;
            newVPartition[end1id] = part1;
            newVPartition[end2id] = part2;

            int candidate = solveInstance(child, usedMask,
                                          vecToPartition(newVPartition, usedMask));
            resultUsed = std::min(resultUsed, candidate + edgeWeight);
        }
    }

    return std::min(resultUnused, resultUsed);
}

int BaseDPSolver::resolveLeafNode(int usedMask) {
    if (usedMask == 1) {
        return 0;
    }
    return INFTY;
}

void BaseDPSolver::printDPState(TreeDecomposition::Node &node,
                                int treeNode, int usedMask, uint64_t partition) {
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
    int idx = 0;
    for (auto i : node.bag) {
        if (usedMask & (1 << idx)) {
            std::cout << " " << i;
        }
        idx++;
    }
    std::cout << "  comps:";
    std::vector<char> vp = partitionToVec((int)node.bag.size(), partition);
    for (idx = 0; idx < (int)node.bag.size(); ++idx) {
        if (usedMask & (1 << idx)) {
            std::cout << " " << (int)vp[idx];
        }
    }
    // std::cout << " Parts: " << std::hex << partition << std::dec;
    std::cout << std::endl;
}

