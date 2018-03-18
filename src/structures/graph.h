#ifndef PACE2018_GRAPH_H
#define PACE2018_GRAPH_H

#include <iostream>
#include <map>
#include <vector>

class Graph {
public:
    Graph() = default;
    void load(std::istream &input);

private:
    int nodes_, edges_, termCount_;
    std::vector<std::map<int, int>> graph_;
    std::vector<bool> is_terminal_;
};


#endif //PACE2018_GRAPH_H
