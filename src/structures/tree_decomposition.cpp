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
    neighbors.clear();
    neighbors.resize((unsigned) nodes);
    for (int i = 0; i < nodes - 1; ++i) {
        int vertA, vertB;
        input >> vertA >> vertB;
        vertA--; vertB--;
        neighbors[vertA].push_back(vertB);
        neighbors[vertB].push_back(vertA);
    }

    // END
    input >> skip;
}
