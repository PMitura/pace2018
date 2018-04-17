#ifndef PACE2018_GRAPH_H
#define PACE2018_GRAPH_H

#include <algorithm>
#include <iostream>
#include <map>
#include <vector>

class Graph {
public:
    Graph() : nodeCount(0), edgeCount(0), termCount(0), edgeWeightSum(0) {}
    void load(std::istream &input);

    bool isTerm(int node) const;
    const std::map<int, int> &getAdjacentOf(int node) const;
    const std::vector<int> &getTerminals() const;
    unsigned long long getEdgeWeightSum() const;

private:
    int nodeCount, edgeCount, termCount;
public:
    int getNodeCount() const;

private:
    unsigned edgeWeightSum;

    std::vector<std::map<int, int>> graph;
    std::vector<bool> is_terminal;
    std::vector<int> terminals;
};


#endif //PACE2018_GRAPH_H
