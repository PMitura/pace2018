#include "graph.h"

void Graph::load(std::istream &input) {
    std::string skip;

    input >> skip >> skip; // skip the "SECTION Graph" part
    input >> skip >> nodes;
    input >> skip >> edges;

    // setup lists of neighbours
    graph.clear();
    graph.resize((unsigned)nodes);

    // load edges
    for (int i = 0; i < edges; i++) {
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
    }
    input >> skip; // END

    // load terminals
    input >> skip >> skip; // skip the "SECTION Terminals" part
    input >> skip >> termCount;
    is_terminal.clear();
    is_terminal.resize((unsigned) termCount);
    for (int i = 0; i < termCount; i++) {
        int termId;
        input >> skip >> termId;
        termId--;
        is_terminal[termId] = true;
    }
    input >> skip; // END
}

bool Graph::isTerm(int node) {
    return is_terminal[node];
}

const std::map<int, int> &Graph::getAdjacentOf(int node) const {
    return graph[node];
}
