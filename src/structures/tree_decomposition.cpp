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
    beautifyDFS(currId, 0, 0, 0, niceNodes);

    nodeCount = currId;
    nodes = niceNodes;
}

void TreeDecomposition::beautifyDFS(int &currId, int uglyNode, int uglyParent, int niceParent, std::vector<Node> &niceNodes) {
    // TODO: edge case, single node in whole decompo
    if (getAdjacentTo(uglyNode).empty()) {
        exit(1);
    }

    bool isRoot = uglyParent == 0;

    // leaf that is not root
    if (getAdjacentTo(uglyNode).size() == 1 && isRoot) {
        std::vector<int> currentBag, reservoir = getBagOf(uglyNode);

        // function to check id of the node above
        auto parentId = [&]() {
            if (reservoir.empty()) return niceParent;
            return currId + 1;
        };

        niceNodes.emplace_back();
        niceNodes[currId].type = LEAF;
        niceNodes[currId].adjacent = {parentId()};
        currId++;

        while (!reservoir.empty()) {
            currentBag.push_back(reservoir.back());
            reservoir.pop_back();
            niceNodes.emplace_back();
            niceNodes[currId].type = INTRO;
            niceNodes[currId].bag = currentBag;
            niceNodes[currId].adjacent = {currId - 1, parentId()};
            currId++;
        }
        return;
    }

    std::vector<int> children = getAdjacentTo(uglyNode);
    children.erase(std::remove(children.begin(), children.end(), uglyParent), children.end());

    int currParent = niceParent;
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
        int chainTail = currId - 1;
        beautifyDFS(currId, children[i], uglyNode, chainTail, niceNodes);

        // attach next branch
        if (i != (int) children.size() - 1) {
            niceNodes[currParent].adjacent.push_back(currId);
        }
    }
}
