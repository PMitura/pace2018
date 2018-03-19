#include "tree_decomposition.h"

void TreeDecomposition::load(std::istream &input) {
    std::string skip;
    input >> skip >> skip >> skip; // skip "SECTION Tree Decomposition

    // read header
    input >> skip;
    if (skip == "c") {
        input.ignore(std::numeric_limits<std::streamsize>::max(), input.widen('\n'));
    }
    input >> skip >> skip >> nodes >> width >> origNodes;
    input.ignore(std::numeric_limits<std::streamsize>::max(), input.widen('\n'));

    // read bags
    bags.clear();
    bags.resize((unsigned) nodes);
    for (int i = 0; i < nodes; ++i) {
        std::string line;
        std::getline(input, line);
        std::istringstream linestream(line);
        linestream >> skip;
        int bagId;
        linestream >> bagId;
        bagId--;
        int content;
        while (linestream >> content) {
            bags[bagId].push_back(content-1);
        }
    }

    // read edges
    adjacent.clear();
    adjacent.resize((unsigned) nodes);
    for (int i = 0; i < nodes - 1; ++i) {
        int vertA, vertB;
        input >> vertA >> vertB;
        vertA--; vertB--;
        adjacent[vertA].push_back(vertB);
        adjacent[vertB].push_back(vertA);
    }

    // initialize node types
    nodeTypes.clear();
    nodeTypes.resize((unsigned) nodes, NOT_NICE);

    // END
    input >> skip;
}

const std::vector<int> &TreeDecomposition::getAdjacentTo(int node) const {
    return adjacent[node];
}

const std::vector<int> &TreeDecomposition::getBagOf(int node) const {
    return bags[node];
}

void TreeDecomposition::convertToNice() {
    std::vector<std::vector<int>> niceAdjacent, niceBags;
    nodeTypes.clear();

    // let the node with id 0 be the root
    int currId = 0;
    beautifyDFS(currId, 0);

    bags = niceBags;
    adjacent = niceAdjacent;
}

int TreeDecomposition::beautifyDFS(int &currId, int uglyNode) {
    return 0;
}
