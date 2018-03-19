#ifndef PACE2018_GRAPH_H
#define PACE2018_GRAPH_H

#include <iostream>
#include <map>
#include <vector>

class Graph {
public:
    Graph() = default;
    void load(std::istream &input);

    bool isTerm(int node);

    int getNodes() const;
    int getEdges() const;
    int getTermCount() const;
    std::map<int, int> &getAdjacentOf(int node);

private:
    int nodes, edges, termCount;
    std::vector<std::map<int, int>> graph;
    std::vector<bool> is_terminal;
};


#endif //PACE2018_GRAPH_H
