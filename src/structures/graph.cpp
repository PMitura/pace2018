#include "graph.h"

void Graph::load(std::istream &input) {
    std::string skip;

    input >> skip >> skip; // skip the "SECTION Graph" part
    input >> skip >> nodeCount;
    input >> skip >> edgeCount;

    // setup lists of neighbours
    graph.clear();
    graph.resize((unsigned)nodeCount);

    edgeWeightSum = 0;
    std::set<std::pair<int, int>> edgeSet;
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
        edgeSet.insert(std::minmax(vertA, vertB));
    }
    std::copy(edgeSet.begin(), edgeSet.end(), std::back_inserter(edgeList));
    input >> skip; // END

    // load terminals
    input >> skip >> skip; // skip the "SECTION Terminals" part
    input >> skip >> termCount;
    isTerminal.clear();
    isTerminal.resize((unsigned)nodeCount, false);
    for (int i = 0; i < termCount; i++) {
        int termId = -1;
        input >> skip >> termId;
        termId--;
        isTerminal[termId] = true;
        terminals.push_back(termId);
    }
    input >> skip; // END

    // preprocess the graph
    isErased.clear();
    isErased.resize((unsigned)nodeCount, false);
    preselectedWeight = 0;
    cutLeaves();

    // round up the graph format
    initEdgeIds();
    std::sort(terminals.begin(), terminals.end());
}

bool Graph::isTerm(int node) const {
    return isTerminal[node];
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


bool Graph::isNodeErased(int id) const {
    return isErased[id];
}

int Graph::getFirstLeaf() {
    for (int i = 0; i < nodeCount; i++) {
        if (!isErased[i] && getAdjacentOf(i).size() == 1) {
            return i;
        }
    }
    return -1;
}

void Graph::cutLeaves() {
    int leaf = getFirstLeaf();
    while (leaf != -1) {
        // get ID of the only neighbor
        int child = (*(getAdjacentOf(leaf).begin())).first;

        // case when the leaf is terminal
        if (isTerm(leaf)) {
            if (!isTerm(child)) {
                isTerminal[child] = true;
                terminals.push_back(child);
            }
            preselectedEdges.emplace_back(std::minmax(leaf, child));
            preselectedWeight += graph[child][leaf];
        }

        // cut the node from the graph
        graph[child].erase(leaf);
        isErased[leaf] = true;

        leaf = getFirstLeaf();
    }

    recomputeStatistics();
}

void Graph::recomputeStatistics() {
    std::vector<int> newTerminals;
    for (int i = 0; i < nodeCount; i++) {
        if (isTerminal[i] && !isErased[i]) {
            newTerminals.push_back(i);
        }
    }
    terminals = newTerminals;
    termCount = (int)terminals.size();

    std::vector<std::pair<int, int>> newEdges;
    for (auto oldEdge : edgeList) {
        if (!isErased[oldEdge.first] || !isErased[oldEdge.second]) {
            newEdges.push_back(oldEdge);
        }
    }
    edgeList = newEdges;
    edgeCount = (int)edgeList.size();
}

std::vector<std::pair<int, int>> Graph::getPreselectedEdges() const {
    return preselectedEdges;
}

int Graph::getPreselectedWeight() const {
    return preselectedWeight;
}

void Graph::initEdgeIds() {
    int currEdgeId = 0;
    for (auto edge : edgeList) {
        if (edgeIds.count(edge) == 0) {
            edgeIds[edge] = currEdgeId++;
        }
    }
}

