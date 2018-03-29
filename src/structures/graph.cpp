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
    for (int i = 0; i < edgeCount; i++) {
        int vertA, vertB, weight;
        input >> skip >> vertA >> vertB >> weight;
        vertA--; vertB--;
        // check for multiedges
        if (graph[vertA].count(vertB)) {
            if (graph[vertA][vertB] <= weight) {
                continue;
            }
        }
        graph[vertA][vertB] = weight;
        graph[vertB][vertA] = weight;
        edgeWeightSum += weight;
    }
    input >> skip; // END

    // load terminals
    input >> skip >> skip; // skip the "SECTION Terminals" part
    input >> skip >> termCount;
    is_terminal.clear();
    is_terminal.resize((unsigned)nodeCount, false);
    for (int i = 0; i < termCount; i++) {
        int termId;
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

unsigned Graph::getEdgeWeightSum() const {
    return edgeWeightSum;
}

