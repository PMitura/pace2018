#include "graph.h"

void Graph::load(std::istream &input) {
    std::string skip;

    input >> skip >> skip; // skip the "SECTION Graph" part
    input >> skip >> nodeCount;
    input >> skip >> edgeCount;

    // setup lists of neighbours
    graph.clear();
    graph.resize((unsigned)nodeCount);

    // load edgeCount
    edgeWeightSum = 0;
    int currEdgeId = 0;
    for (int i = 0; i < edgeCount; i++) {
        int vertA = -1, vertB = -1, weight = -1;
        input >> skip >> vertA >> vertB >> weight;
        vertA--; vertB--;
        // check for multiedges
        if (graph[vertA].count(vertB) != 0) {
            if (graph[vertA][vertB] <= weight) {
                continue;
            }
        }
        graph[vertA][vertB] = weight;
        graph[vertB][vertA] = weight;
        edgeWeightSum += weight;

        std::pair<int, int> edge = std::minmax(vertA, vertB);
        if (edgeIds.count(edge) == 0) {
            edgeIds[edge] = currEdgeId++;
            edgeList.push_back(edge);
        }
    }
    input >> skip; // END

    // load terminals
    input >> skip >> skip; // skip the "SECTION Terminals" part
    input >> skip >> termCount;
    is_terminal.clear();
    is_terminal.resize((unsigned)nodeCount, false);
    for (int i = 0; i < termCount; i++) {
        int termId = -1;
        input >> skip >> termId;
        termId--;
        is_terminal[termId] = true;
        terminals.push_back(termId);
    }
    std::sort(terminals.begin(), terminals.end());
    input >> skip; // END
}

bool Graph::isTerm(int node) const {
    return is_terminal[node];
}

const std::map<int, int> &Graph::getAdjacentOf(int node) const {
    return graph[node];
}

const std::vector<int> &Graph::getTerminals() const {
    return terminals;
}

unsigned long long Graph::getEdgeWeightSum() const {
    return edgeWeightSum;
}

int Graph::getNodeCount() const {
    return nodeCount;
}

int Graph::idOfEdge(const std::pair<int, int>& edge) const {
    return edgeIds.at(edge);
}

std::pair<int, int> Graph::edgeWithId(int id) const {
    return edgeList[id];
}

int Graph::getEdgeCount() const {
    return edgeCount;
}

