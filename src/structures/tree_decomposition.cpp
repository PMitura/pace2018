#include "tree_decomposition.h"

void TreeDecomposition::load(std::istream &input) {
    std::string skip;
    input >> skip >> skip >> skip; // skip "SECTION Tree Decomposition

    // read header
    input >> skip;
    if (skip == "c") {
        input.ignore(std::numeric_limits<std::streamsize>::max(), input.widen('\n'));
        input >> skip;
    }
    input >> skip >> nodeCount >> width >> origNodes;
    input.ignore(std::numeric_limits<std::streamsize>::max(), input.widen('\n'));
    nodes.clear();
    nodes.resize(nodeCount);

    // read bags
    for (unsigned i = 0; i < nodeCount; ++i) {
        std::string line;
        std::getline(input, line);
        std::istringstream linestream(line, std::ios_base::in);
        linestream >> skip;
        int bagId;
        linestream >> bagId;
        bagId--;
        int content;
        while (linestream >> content) {
            nodes[bagId].bag.push_back(content-1);
        }
    }

    // read edgeCount
    for (unsigned i = 0; i < nodeCount - 1; ++i) {
        int vertA, vertB = -1;
        input >> vertA >> vertB;
        vertA--; vertB--;
        nodes[vertA].adjacent.push_back(vertB);
        nodes[vertB].adjacent.push_back(vertA);
    }

    // END
    input >> skip;
}

const std::vector<int> &TreeDecomposition::getAdjacentTo(int node) const {
    return nodes[node].adjacent;
}

const std::vector<int> &TreeDecomposition::getBagOf(int node) const {
    return nodes[node].bag;
}

void TreeDecomposition::convertToNice(const Graph &sourceGraph) {
    std::vector<Node> niceNodes;
    enabledEdges.clear();

    // find leaf to be the new root
    int uglyRoot = 0;
    for (auto node : nodes) {
        if (node.adjacent.size() == 1) {
            break;
        }
        uglyRoot++;
    }
    int currId = 0;
    beautifyDFS(currId, uglyRoot, -1, niceNodes, sourceGraph);

    nodeCount = (unsigned)currId;
    nodes = niceNodes;
    for (unsigned i = 0; i < nodeCount; ++i) {
        std::sort(nodes[i].bag.begin(), nodes[i].bag.end());
    }
}

void TreeDecomposition::addNodeEverywhere(int nodeId) {
    for (auto& node : nodes) {
        if (!std::binary_search(node.bag.begin(), node.bag.end(), nodeId)) {
            node.bag.push_back(nodeId);
            std::sort(node.bag.begin(), node.bag.end());
        }
    }
}

void TreeDecomposition::beautifyDFS(int &currId,
                                    int uglyNode,
                                    int uglyParent,
                                    std::vector<Node> &niceNodes,
                                    const Graph &graph) {
    // TODO: edge case, single node in whole decompo
    if (getAdjacentTo(uglyNode).empty()) {
        exit(1);
    }

    bool isRoot = (uglyParent == -1);

    // leaf that is not root
    if (getAdjacentTo(uglyNode).size() == 1 && !isRoot) {
        std::vector<int> reservoir = getBagOf(uglyNode);

        while (!reservoir.empty()) {
            niceNodes.emplace_back();
            niceNodes[currId].type = INTRO;
            niceNodes[currId].associatedNode = reservoir.back();
            niceNodes[currId].bag = reservoir;
            niceNodes[currId].adjacent = {currId - 1, currId + 1};
            reservoir.pop_back();
            currId++;
        }

        niceNodes.emplace_back();
        niceNodes[currId].type = LEAF;
        niceNodes[currId].adjacent = {currId - 1};
        currId++;
        return;
    }

    std::vector<int> children = getAdjacentTo(uglyNode);
    if (!isRoot) {
        children.erase(std::remove(children.begin(), children.end(), uglyParent), children.end());
    }

    // create chain to the root bag
    if (isRoot && !getBagOf(uglyNode).empty()) {
        // root leaf
        std::vector<int> targetBag = getBagOf(uglyNode), currentBag;
        currentBag.push_back(targetBag.back());

        niceNodes.emplace_back();
        niceNodes[currId].type = FORGET;
        niceNodes[currId].adjacent = {currId + 1};
        niceNodes[currId].associatedNode = targetBag.back();
        currId++;

        addIntroEdgesOfNode(currId, targetBag.back(), currentBag, graph, niceNodes);
        while (targetBag.size() != 1) {
            targetBag.pop_back();
            niceNodes.emplace_back();
            niceNodes[currId].type = FORGET;
            niceNodes[currId].bag = currentBag;
            niceNodes[currId].associatedNode = targetBag.back();
            niceNodes[currId].adjacent = {currId - 1, currId + 1};
            currId++;
            currentBag.push_back(targetBag.back());
            addIntroEdgesOfNode(currId, targetBag.back(), currentBag, graph, niceNodes);
        }
    }

    int currParent = currId - 1;
    for (int i = 0; i < (int) children.size(); i++) {
        // create new JOIN root
        std::vector<int> targetBag = getBagOf(children[i]), currentBag = getBagOf(uglyNode);
        if (i != (int) children.size() - 1) {
            niceNodes.emplace_back();
            niceNodes[currId].type = JOIN;
            if (!isRoot) {
                niceNodes[currId].adjacent.push_back(currParent);
            }
            niceNodes[currId].bag = getBagOf(uglyNode);
            currParent = currId;
            currId++;
            isRoot = false;
        }

        // attach i-th child branch
        if (i != (int) children.size() - 1) {
            niceNodes[currParent].adjacent.push_back(currId);
        }
        // create a nice path fron JOIN to i-th child
        std::vector<int> intersect, exCurr, exTarget, targetReservoir;
        divide(currentBag, targetBag, intersect, exCurr, exTarget);
        while (!exCurr.empty()) {
            niceNodes.emplace_back();
            niceNodes[currId].type = INTRO;
            niceNodes[currId].bag = intersect;
            for (auto item : exCurr) {
                niceNodes[currId].bag.push_back(item);
            }
            niceNodes[currId].associatedNode = exCurr.back();
            niceNodes[currId].adjacent = {currId - 1, currId + 1};
            currId++;
            exCurr.pop_back();
        }
        while (!exTarget.empty()) {
            niceNodes.emplace_back();
            niceNodes[currId].type = FORGET;
            niceNodes[currId].bag = intersect;
            niceNodes[currId].associatedNode = exTarget.back();
            // empty root case
            if (isRoot) {
                niceNodes[currId].adjacent = {currId + 1};
                isRoot = false;
            } else {
                niceNodes[currId].adjacent = {currId - 1, currId + 1};
            }
            currId++;
            intersect.push_back(exTarget.back());
            addIntroEdgesOfNode(currId, exTarget.back(), intersect, graph, niceNodes);
            exTarget.pop_back();
        }

        // attach child subtree
        beautifyDFS(currId, children[i], uglyNode, niceNodes, graph);

        // attach next branch
        if (i != (int) children.size() - 1) {
            niceNodes[currParent].adjacent.push_back(currId);
        }
    }
}

void TreeDecomposition::printTree(std::ostream &output) {
    int nodeId = 0;
    for (auto &node : nodes) {
        output << "Node ID: " << nodeId++;
        output << "  Adjacent:";
        for (auto adj : node.adjacent) {
            output << " " << adj;
        }
        output << "  Bag:";
        std::vector<int> bag = node.bag;
        for (auto item : bag) {
            output << " " << item;
        }
        output << "  Type: ";
        switch(node.type) {
            case INTRO:
                output << "INTRO";
                break;
            case FORGET:
                output << "FORGET";
                break;
            case JOIN:
                output << "JOIN";
                break;
            case INTRO_EDGE:
                output << "INTRO_EDGE";
                break;
            case LEAF:
                output << "LEAF";
                break;
            default:
                output << "!!! NOT NICE";
        }
        if (node.type == INTRO || node.type == FORGET) {
            output << "  Assoc. node: " << node.associatedNode;
        }
        if (node.type == INTRO_EDGE) {
            output << "  Assoc. edge: " << node.associatedEdge.first
                   << " - " << node.associatedEdge.second;
        }
        output << std::endl;
    }
}

void TreeDecomposition::addIntroEdgesOfNode(int &currId,
                                            int node,
                                            const std::vector<int> &bag,
                                            const Graph &graph,
                                            std::vector<Node> &niceNodes) {
    for (auto adj : graph.getAdjacentOf(node)) {
        std::pair<int, int> edge = std::minmax(node, adj.first);
        if (enabledEdges.count(edge) == 0) {
            enabledEdges.insert(edge);
            continue;
        }

        niceNodes.emplace_back();
        niceNodes[currId].type = INTRO_EDGE;
        niceNodes[currId].bag = bag;
        niceNodes[currId].associatedEdge = edge;
        niceNodes[currId].adjacent = {currId - 1, currId + 1};
        currId++;
    }
}

unsigned TreeDecomposition::getNodeCount() const {
    return nodeCount;
}

const TreeDecomposition::Node &TreeDecomposition::getNodeAt(int id) const {
    return nodes[id];
}


