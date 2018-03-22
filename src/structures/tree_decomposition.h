#ifndef PACE2018_TREEDECOMPOSITION_H
#define PACE2018_TREEDECOMPOSITION_H

#include <algorithm>
#include <istream>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "utility/helpers.h"
#include "structures/graph.h"

class TreeDecomposition {
public:
    TreeDecomposition() : nodeCount(0), width(0), origNodes(0) {}

    void load(std::istream &input);
    const std::vector<int> &getAdjacentTo(int node) const;
    const std::vector<int> &getBagOf(int node) const;

    void convertToNice(const Graph &sourceGraph);

    void printTree(std::ostream& output);

    enum NodeType {NOT_NICE, INTRO, FORGET, JOIN, INTRO_EDGE, LEAF};

private:
    struct Node {
        Node() : type(NOT_NICE), associatedNode(-1) {}
        std::vector<int> bag, adjacent;
        NodeType type;
        int associatedNode;
        std::pair<int, int> associatedEdge;
    };

    void beautifyDFS(int &currId,
                     int uglyNode,
                     int uglyParent,
                     std::vector<Node> &niceNodes,
                     const Graph &graph);

    void addIntroEdgesOfNode(int &currId,
                             int node,
                             const std::vector<int> &bag,
                             const Graph &graph,
                             std::vector<Node> &niceNodes);

    int nodeCount, width, origNodes;
    std::vector<Node> nodes;
    std::set<std::pair<int, int>> introducedEdges;
};


#endif //PACE2018_TREEDECOMPOSITION_H
