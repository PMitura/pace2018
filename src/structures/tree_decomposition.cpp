#include "tree_decomposition.h"

void TreeDecomposition::load(std::istream &input) {
    std::string skip;
    input >> skip >> skip >> skip; // skip "SECTION Tree Decomposition

    // read header
    input >> skip;
    if (skip == "c") {
        input.ignore(std::numeric_limits<std::streamsize>::max(), input.widen('\n'));
    }
    input >> skip >> skip >> nodeCount >> width >> origNodes;
    input.ignore(std::numeric_limits<std::streamsize>::max(), input.widen('\n'));
    nodes.clear();
    nodes.resize((unsigned) nodeCount);

    // read bags
    for (int i = 0; i < nodeCount; ++i) {
        std::string line;
        std::getline(input, line);
        std::istringstream linestream(line);
        linestream >> skip;
        int bagId;
        linestream >> bagId;
        bagId--;
        int content;
        while (linestream >> content) {
            nodes[bagId].bag.push_back(content-1);
        }
    }

    // read edges
    for (int i = 0; i < nodeCount - 1; ++i) {
        int vertA, vertB;
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

void TreeDecomposition::convertToNice() {
    std::vector<Node> niceNodes;

    int currId = 0;
    beautifyDFS(currId, 0, 0, niceNodes);

    nodeCount = currId;
    nodes = niceNodes;
}

void TreeDecomposition::beautifyDFS(int &currId, int uglyNode, int uglyParent, std::vector<Node> &niceNodes) {
    // TODO: edge case, single node in whole decompo
    if (getAdjacentTo(uglyNode).empty()) {
        exit(1);
    }

    // leaf that is not root
    if (getAdjacentTo(uglyNode).size() == 1 && currId) {
        std::vector<int> reservoir = getBagOf(uglyNode);

        while (!reservoir.empty()) {
            niceNodes.emplace_back();
            niceNodes[currId].type = INTRO;
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
    children.erase(std::remove(children.begin(), children.end(), uglyParent), children.end());

    int currParent = currId - 1;
    for (int i = 0; i < (int) children.size(); i++) {
        // create new JOIN root
        std::vector<int> targetBag = getBagOf(children[i]), currentBag = getBagOf(uglyNode);
        if (i != (int) children.size() - 1) {
            niceNodes.emplace_back();
            niceNodes[currId].type = JOIN;
            if (currId) {
                niceNodes[currId].adjacent.push_back(currParent);
            }
            niceNodes[currId].bag = getBagOf(uglyNode);
            currParent = currId;
            currId++;
        }

        // create a nice path fron JOIN to i-th child
        niceNodes[currParent].adjacent.push_back(currId);
        std::vector<int> intersect, exCurr, exTarget, targetReservoir;
        divide(currentBag, targetBag, intersect, exCurr, exTarget);
        while (!exCurr.empty()) {
            niceNodes.emplace_back();
            niceNodes[currId].type = INTRO;
            niceNodes[currId].bag = intersect;
            for (auto item : exCurr) {
                niceNodes[currId].bag.push_back(item);
            }
            niceNodes[currId].adjacent = {currId - 1, currId + 1};
            currId++;
            exCurr.pop_back();
        }
        while (!exTarget.empty()) {
            niceNodes.emplace_back();
            niceNodes[currId].type = FORGET;
            niceNodes[currId].bag = intersect;
            niceNodes[currId].adjacent = {currId - 1, currId + 1};
            currId++;
            intersect.push_back(exTarget.back());
            exTarget.pop_back();
        }

        // attach child subtree
        beautifyDFS(currId, children[i], uglyNode, niceNodes);

        // attach next branch
        if (i < (int) children.size() - 2) {
            niceNodes[currParent].adjacent.push_back(currId);
        }
    }
}

void TreeDecomposition::printTree(std::ostream &output) {
    int nodeId = 0;
    for (auto &node : nodes) {
        output << "Node ID: " << nodeId++ << std::endl;
        output << "  Adjacent:";
        for (auto adj : node.adjacent) {
            output << " " << adj;
        }
        output << std::endl << "  Bag:";
        std::vector<int> bag = node.bag;
        std::sort(bag.begin(), bag.end());
        for (auto item : bag) {
            output << " " << item;
        }
        output << std::endl << "  Type: ";
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
        output << std::endl << std::endl;
    }
}
