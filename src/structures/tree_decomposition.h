#ifndef PACE2018_TREEDECOMPOSITION_H
#define PACE2018_TREEDECOMPOSITION_H

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

    enum NodeType {NOT_NICE, INTRO, FORGET, JOIN, INTRO_EDGE};

private:
    int beautifyDFS(int &currId, int uglyNode);

    int nodes, width, origNodes;

    std::vector<std::vector<int>> bags, adjacent;
    std::vector<NodeType> nodeTypes;
};


#endif //PACE2018_TREEDECOMPOSITION_H
