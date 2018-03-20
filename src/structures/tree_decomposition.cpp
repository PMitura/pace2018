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

    // let the node with id 0 be the root
    int currId = 0;
    beautifyDFS(currId, 0, 0, 0, niceNodes);
    nodeCount = currId;

}

int TreeDecomposition::beautifyDFS(int &currId, int uglyNode, int uglyParent, int niceParent, std::vector<Node> &niceNodes) {
    // TODO: edge case, single node in whole decompo
    if (getAdjacentTo(uglyNode).empty()) {
        exit(1);
    }

    // leaf that is not root
    if (getAdjacentTo(uglyNode).size() == 1 && uglyNode) {
        std::vector<int> currentBag, reservoir = getBagOf(uglyNode);

        // function to check id of the node above
        auto parentId = [&]() {
            if (reservoir.empty()) return uglyParent;
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
        return currId - 1;
    }

    std::vector<int> children = getAdjacentTo(uglyNode);
    children.erase(std::remove(children.begin(), children.end(), uglyParent), children.end());

    // function to create path between nodes with two different bags
    auto makePath = [&](std::vector<int> &bagFrom, std::vector<int> &bagTo) {
        std::vector<int> bagIntersection;
    };

    // int subtree = beautifyDFS(currId, children[0], uglyNode, 0, niceNodes);

    for (int i = 1; i < (int) children.size(); i++) {
    }

    return 0;
}
