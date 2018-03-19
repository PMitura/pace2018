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

private:
    int nodes, width, origNodes;

    std::vector<std::vector<int>> bags, neighbors;
};


#endif //PACE2018_TREEDECOMPOSITION_H
