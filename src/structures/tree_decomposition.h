#ifndef PACE2018_TREEDECOMPOSITION_H
#define PACE2018_TREEDECOMPOSITION_H

#include <algorithm>
#include <istream>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <vector>

class TreeDecomposition {
public:
    void load(std::istream &input);
    const std::vector<int> &getAdjacentTo(int node) const;
    const std::vector<int> &getBagOf(int node) const;

    void convertToNice();

    enum NodeType {NOT_NICE, INTRO, FORGET, JOIN, INTRO_EDGE, LEAF};

private:
    struct Node {
        Node() : type(NOT_NICE) {}
        std::vector<int> bag, adjacent;
        NodeType type;
    };

    int beautifyDFS(int &currId, int uglyNode, int uglyParent, int niceParent, std::vector<Node> &niceNodes);

    int nodeCount, width, origNodes;
    std::vector<Node> nodes;
};


#endif //PACE2018_TREEDECOMPOSITION_H
