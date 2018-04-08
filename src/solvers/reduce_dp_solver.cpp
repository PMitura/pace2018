#include "reduce_dp_solver.h"

Graph ReduceDPSolver::solve() {
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
    std::cout << "INTRO time  " << (double)introTime / CLOCKS_PER_SEC   << "s" << std::endl;
    std::cout << "FORGET time " << (double)forgetTime / CLOCKS_PER_SEC  << "s" << std::endl;
    std::cout << "JOIN time   " << (double)joinTime / CLOCKS_PER_SEC    << "s" << std::endl;
    std::cout << "EDGE time   " << (double)edgeTime / CLOCKS_PER_SEC    << "s" << std::endl;

    std::cout << "PART time   " << (double)partTime / CLOCKS_PER_SEC    << "s" << std::endl;
    std::cout << "REDUCE time " << (double)matrixTime / CLOCKS_PER_SEC  << "s" << std::endl;
     */

    return Graph();
}

void ReduceDPSolver::initializeDP() {
    unsigned treeNodes = decomposition.getNodeCount();
    dpCache.resize(treeNodes);
    dpBacktrack.resize(treeNodes);
    joinBacktrack.resize(treeNodes);
    for (unsigned i = 0; i < treeNodes; ++i) {
        auto bagSize = decomposition.getBagOf(i).size();
        // 64b variable insufficient for partitions
        if (bagSize > 16) {
            std::cerr << "Error: bags too large!" << std::endl;
            exit(1);
        }

        dpCache[i].resize(1u << bagSize);
        dpBacktrack[i].resize(1u << bagSize);
        joinBacktrack[i].resize(1u << bagSize);
    }
    resultEdges.clear();
}

void ReduceDPSolver::backtrack(int treeNode, unsigned subset, uint64_t partition) {
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

void ReduceDPSolver::solveForSubset(unsigned nodeId, unsigned subset) {
    TreeDecomposition::Node node = decomposition.getNodeAt(nodeId);

    clock_t startClock = clock();
    std::vector<uint64_t> partitions = generateParts(nodeId, subset);
    partTime += (clock() - startClock);

    // JOIN and EDGE are forwarded by generator
    if (node.type == TreeDecomposition::INTRO || node.type == TreeDecomposition::LEAF || node.type == TreeDecomposition::FORGET) {
        for (auto part : partitions) {
            solveForPartition(node, nodeId, subset, part);
        }
    }

    // reduce the number of partitions
    startClock = clock();
    reduce(nodeId, subset);
    matrixTime += (clock() - startClock);
}

void ReduceDPSolver::reduce(unsigned nodeId, unsigned subset) {
    TreeDecomposition::Node node = decomposition.getNodeAt(nodeId);

    std::vector<uint64_t> partitions;

    for (auto entry : dpCache[nodeId][subset]) {
        if (entry.second >= INFTY) {
            continue;
        }
        partitions.push_back(entry.first);
    }

    std::sort(partitions.begin(), partitions.end(), [&](const uint64_t& a, const uint64_t& b) {
        return dpCache[nodeId][subset][a] < dpCache[nodeId][subset][b];
    });

    std::vector<uint64_t> reducedPartitions;
    if ((partitions.size() << 1u) > (1u << (unsigned)(__builtin_popcount(subset)))) {
        CutMatrix cutMatrix;
        cutMatrix.generate(partitions, subset, (unsigned) node.bag.size());
        cutMatrix.eliminate();
        reducedPartitions = cutMatrix.getPartitions();
        /*
        std::cout << "  ORIGINAL: " << partitions.size() << std::endl;
        std::cout << "  REDUCED:  " << reducedPartitions.size() << std::endl;
         */
    } else {
        reducedPartitions = partitions;
    }


    std::unordered_map<uint64_t, unsigned> newPart;
    for (auto part : reducedPartitions) {
        newPart[part] = dpCache[nodeId][subset][part];
    }
    dpCache[nodeId][subset] = newPart;
}

void ReduceDPSolver::solveForPartition(TreeDecomposition::Node &node,
                                      int nodeId, unsigned subset, uint64_t partition) {
    unsigned result;
    clock_t startClock = clock();
    switch (node.type) {
        case TreeDecomposition::INTRO:
            result = resolveIntroNode(node, nodeId, subset, partition);
            introTime += (clock() - startClock);
            break;
        case TreeDecomposition::FORGET:
            result = resolveForgetNode(node, nodeId, subset, partition);
            forgetTime += (clock() - startClock);
            break;
        case TreeDecomposition::JOIN:
            result = resolveJoinNode(node, nodeId, subset, partition);
            joinTime += (clock() - startClock);
            break;
        case TreeDecomposition::INTRO_EDGE:
            result = resolveEdgeNode(node, nodeId, subset, partition);
            edgeTime += (clock() - startClock);
            break;
        case TreeDecomposition::LEAF:
            result = resolveLeafNode(subset);
            leafTime += (clock() - startClock);
            break;
        default:
            std::cerr << "Error, decomposition not nice!" << std::endl;
            exit(1);
    }
    dpCache[nodeId][subset][partition] = result;
}

unsigned ReduceDPSolver::getFromCache(int nodeId, unsigned subset, uint64_t partition) {
    if (dpCache[nodeId][subset].count(partition) != 0) {
        return dpCache[nodeId][subset][partition];
    }
    return INFTY;
}

unsigned ReduceDPSolver::resolveIntroNode(TreeDecomposition::Node &node,
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

unsigned ReduceDPSolver::resolveForgetNode(TreeDecomposition::Node &node,
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
        newMask = maskWithElement(subset, (unsigned)forgottenId, 0, (int)node.bag.size());
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

unsigned ReduceDPSolver::resolveJoinNode(TreeDecomposition::Node &node,
                                        int treeNode, unsigned int subset, uint64_t partition) {
    // get children IDs
    auto bagSize = node.bag.size();
    int children[2], childPtr = 0;
    for (auto adj : node.adjacent) {
        if (adj < treeNode) {
            continue;
        }
        children[childPtr++] = adj;
    }

    // find possible partitions
    std::vector<uint64_t> subpartitions1, subpartitions2;
    for (auto entry : dpCache[children[0]][subset]) {
        subpartitions1.push_back(entry.first);
    }
    for (auto entry : dpCache[children[1]][subset]) {
        subpartitions2.push_back(entry.first);
    }

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
    for (auto i : subpartitions1) {
        int compCount = maxComponentIn(i, (unsigned)bagSize) + 1;
        results1.push_back({getFromCache(children[0], subset, i), compCount, i});
    }
    for (auto i : subpartitions2) {
        int compCount = maxComponentIn(i, (unsigned)bagSize) + 1;
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

unsigned ReduceDPSolver::resolveEdgeNode(TreeDecomposition::Node &node,
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

unsigned ReduceDPSolver::resolveLeafNode(unsigned int subset) {
    if (subset == 1) {
        return 0;
    }
    return INFTY;
}

std::vector<uint64_t> ReduceDPSolver::generateIntroParts(int nodeId, unsigned subset, uint64_t sourcePart) {
    TreeDecomposition::Node node = decomposition.getNodeAt(nodeId);

    // get the id of the introduced node
    int introduced = node.associatedNode;
    if (introduced == globalTerminal) {
        return {sourcePart};
    }
    unsigned introducedId = 0;
    while (node.bag[introducedId] != introduced) {
        introducedId++;
    }

    // find new partition ID
    char newPartitionId = 0;
    std::vector<char> vPartition = partitionToVec((unsigned)node.bag.size() - 1, sourcePart);
    if (isInSubset(introducedId, subset)) {
        newPartitionId = *std::max_element(vPartition.begin(), vPartition.end()) + (char)1;
    }

    // assign the new partition introduced node
    vPartition.insert(vPartition.begin() + introducedId, newPartitionId);
    return {vecToPartition(vPartition, subset)};
}

std::vector<uint64_t> ReduceDPSolver::generateForgetParts(int nodeId, unsigned subset, uint64_t sourcePart,
                                                          unsigned childSubset) {
    TreeDecomposition::Node node = decomposition.getNodeAt(nodeId);

    // get the singular child
    int child = 0;
    for (auto adj : node.adjacent) {
        if (adj < nodeId) {
            continue;
        }
        child = adj;
    }
    const TreeDecomposition::Node &childNode = decomposition.getNodeAt(child);

    // get id of the forgotten node in child
    int forgotten = node.associatedNode, forgottenId = 0;
//    unsigned candidate = dpCache[child][childSubset][sourcePart];
    if (forgotten == globalTerminal) {
        /*
            if (dpCache[nodeId][subset].count(sourcePart) == 0
                || candidate < dpCache[nodeId][subset][sourcePart]) {
                dpCache[nodeId][subset][sourcePart] = candidate;
                dpBacktrack[nodeId][subset][sourcePart] = {child, childSubset, sourcePart};
            }
         */
            return {sourcePart};
    }

    while (childNode.bag[forgottenId] != forgotten) {
        forgottenId++;
    }
    uint64_t parentPartition
            = partitionWithoutElement(partitionToVec((unsigned)childNode.bag.size(),
                                                     sourcePart),
                                      forgottenId, subset);

    /*
    // forward the results
    if (dpCache[nodeId][subset].count(parentPartition) == 0
        || candidate < dpCache[nodeId][subset][parentPartition]) {
        dpCache[nodeId][subset][parentPartition] = candidate;
        dpBacktrack[nodeId][subset][parentPartition] = {child, childSubset, sourcePart};
    }
     */

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
                dpBacktrack[nodeId][subset][merged]   = {children[0], subset, p1};
                joinBacktrack[nodeId][subset][merged] = {children[1], subset, p2};
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
    int child = 0;
    for (auto adj : node.adjacent) {
        if (adj < nodeId) {
            continue;
        }
        child = adj;
    }
    std::vector<uint64_t> partitions;

    // case where we don't use the edge, forward the result to the cache
    partitions.push_back(sourcePart);
    unsigned candidate = dpCache[child][subset][sourcePart];
    if (dpCache[nodeId][subset].count(sourcePart) == 0
        || dpCache[nodeId][subset][sourcePart] > candidate) {
        dpCache[nodeId][subset][sourcePart] = candidate;
        dpBacktrack[nodeId][subset][sourcePart] = {child, subset, sourcePart};
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
            dpBacktrack[nodeId][subset][newPart] = {child, subset, sourcePart};
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
        Partitioner partitioner(0, subset, (unsigned)node.bag.size());
        partitioner.compute();
        return partitioner.getResult();
    }

    std::unordered_set<uint64_t> setResult;

    if (node.type == TreeDecomposition::INTRO) {
        int introduced = node.associatedNode;
        unsigned childSubset = subset;
        if (introduced != globalTerminal) {
            unsigned introducedId = 0;
            while (node.bag[introducedId] != introduced) {
                introducedId++;
            }
            childSubset = maskWithoutElement(subset, introducedId, (unsigned)node.bag.size());
        }
        for (auto i : dpCache[children[0]][childSubset]) {
            std::vector<uint64_t> generatedByPart = generateIntroParts(nodeId, subset, i.first);
            for (auto part : generatedByPart) {
                setResult.insert(part);
            }
        }
    }

    if (node.type == TreeDecomposition::FORGET) {
        const TreeDecomposition::Node &childNode = decomposition.getNodeAt(children[0]);

        // get id of the forgotten node in child
        int forgotten = node.associatedNode;
        unsigned childSubset1 = subset, childSubset2 = subset, forgottenId = 0;
        if (forgotten != globalTerminal) {
            while (childNode.bag[forgottenId] != forgotten) {
                forgottenId++;
            }
            childSubset1 = maskWithElement(subset, forgottenId, 0, (unsigned)node.bag.size());
            childSubset2 = maskWithElement(subset, forgottenId, 1, (unsigned)node.bag.size());
        }

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

