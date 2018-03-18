#include "graph.h"

void Graph::load(std::istream &input) {
    std::string skip;

    input >> skip >> skip; // skip the "SECTION Graph" part
    input >> skip >> nodes_;
    input >> skip >> edges_;

    // setup lists of neighbours
    graph_.clear();
    graph_.resize((unsigned)nodes_);

    // load edges
    for (int i = 0; i < edges_; i++) {
        int vertA, vertB, weight;
        input >> skip >> vertA >> vertB >> weight;
        // check for multiedges
        if (graph_[vertA].count(vertB)) {
            if (graph_[vertA][vertB] <= weight) {
                continue;
            }
        }
        graph_[vertA][vertB] = weight;
        graph_[vertB][vertA] = weight;
    }
    input >> skip; // END

    // load terminals
    input >> skip >> skip; // skip the "SECTION Terminals" part
    input >> skip >> termCount_;
    is_terminal_.clear();
    is_terminal_.resize((unsigned) termCount_);
    for (int i = 0; i < termCount_; i++) {
        int termId;
        input >> termId;
        is_terminal_[termId] = true;
    }
    input >> skip; // END
}
