#ifndef PACE2018_GRAPH_H
#define PACE2018_GRAPH_H

#include <iostream>
#include <map>
#include <vector>

class Graph {
public:
    Graph() : nodes(0), edges(0), termCount(0) {}
    void load(std::istream &input);

    bool isTerm(int node);

    const std::map<int, int> &getAdjacentOf(int node) const;

private:
    int nodes, edges, termCount;
    std::vector<std::map<int, int>> graph;
    std::vector<bool> is_terminal;

};


#endif //PACE2018_GRAPH_H
