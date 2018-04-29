#ifndef PACE2018_GRAPH_H
#define PACE2018_GRAPH_H

#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <vector>

class Graph {
public:
    Graph() : nodeCount(0), edgeCount(0), termCount(0), edgeWeightSum(0) {}
    void load(std::istream &input);

    int idOfEdge(const std::pair<int, int>& edge) const;
    std::pair<int, int> edgeWithId(int id) const;

    bool isTerm(int node) const;
    const std::map<int, int> &getAdjacentOf(int node) const;
    const std::vector<int> &getTerminals() const;
    unsigned long long getEdgeWeightSum() const;
    int getNodeCount() const;
    int getEdgeCount() const;
    bool isNodeErased(int id) const;
    std::vector<std::pair<int, int>> getPreselectedEdges() const;
    int getPreselectedWeight() const;

private:
    int getFirstLeaf();
    void cutLeaves();
    void recomputeStatistics();
    void initEdgeIds();

    int nodeCount, edgeCount, termCount;
    unsigned edgeWeightSum, preselectedWeight;

    std::vector<std::map<int, int>> graph;
    std::map<std::pair<int, int>, int> edgeIds;
    std::vector<std::pair<int, int>> edgeList, preselectedEdges;
    std::vector<bool> isTerminal, isErased;
    std::vector<int> terminals;
};


#endif //PACE2018_GRAPH_H
