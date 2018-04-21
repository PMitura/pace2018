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

    int idOfEdge(const std::pair<int, int> &edge);
    std::pair<int, int> edgeWithId(int id) const;

    bool isTerm(int node) const;
    const std::map<int, int> &getAdjacentOf(int node) const;
    const std::vector<int> &getTerminals() const;
    unsigned long long getEdgeWeightSum() const;
    int getNodeCount() const;

private:
    int nodeCount, edgeCount, termCount;
    unsigned edgeWeightSum;

    std::vector<std::map<int, int>> graph;
    std::map<std::pair<int, int>, int> edgeIds;
    std::vector<std::pair<int, int>> edgeList;
    std::vector<bool> is_terminal;
    std::vector<int> terminals;
};


#endif //PACE2018_GRAPH_H
