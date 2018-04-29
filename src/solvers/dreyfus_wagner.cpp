#include "dreyfus_wagner.h"

Graph DreyfusWagner::solve() {
//     std::cout << "Using Dreyfus" << std::endl;

    std::vector<int> terminals = graph.getTerminals();
    unsigned k = (unsigned)terminals.size(), n = (unsigned)graph.getNodeCount();

    // intialize dynamic programming caches
    dp = new unsigned*[1u << k];
    dp_par = new unsigned*[1u << k];
    for (int i = 0; i < (1 << k); i++) {
        dp[i] = new unsigned[n];
        for (unsigned j = 0; j < n; j++) {
            dp[i][j] = i ? INFTY : 0;
        }
        dp_par[i] = new unsigned[n];
    }
    closed = new int[n];
    parent = new int[n];
    dist   = new unsigned[n];

    // initial values of subsets only containing one terminal
    int enumerate = 0;
    for (auto i : terminals) {
        dp[1u << enumerate][i] = 0;
        enumerate++;
    }

    // for all subsets of terminals
    for (unsigned subset = 1; subset < (1u << k); subset++) {
        unsigned most_sig = (1u << 31u) >> (unsigned)__builtin_clz(subset);
        for (unsigned d = (subset - 1) & subset; d & most_sig; d = (d - 1) & subset) {
            for (unsigned root = 0; root < n; root++) {
                if (dp[subset][root] > dp[d][root] + dp[subset - d][root]) {
                    dp[subset][root] = dp[d][root] + dp[subset - d][root];
                    dp_par[subset][root] = d;
                }
            }
        }

        memset(closed, 0, sizeof(int)*n);
        std::priority_queue<std::pair<unsigned, unsigned>, std::vector<std::pair<unsigned, unsigned>>,
                std::greater<std::pair<unsigned, unsigned>>> dijkstra_q;
        for (unsigned i = 0; i < n; i++) {
            dijkstra_q.push({dp[subset][i], i});
            dist[i] = dp[subset][i];
        }
        while (dijkstra_q.size() != 0) {
            std::pair<unsigned, unsigned> curr = dijkstra_q.top();
            dijkstra_q.pop();
            if (closed[curr.second] != 0) {
                continue;
            }
            closed[curr.second] = 1;
            for (auto adj : graph.getAdjacentOf(curr.second)) {
                if (closed[adj.first] != 0) {
                    continue;
                }
                if (dist[adj.first] > curr.first + adj.second) {
                    dist[adj.first] = curr.first + adj.second;
                    dijkstra_q.push({dist[adj.first], adj.first});
                }
            }
        }

        for (unsigned i = 0; i < n; i++) {
            dp[subset][i] = dist[i];
        }
    }

    std::cout << "VALUE " << dp[(1u << k) - 1][terminals[0]] + graph.getPreselectedWeight() << std::endl;
    std::vector<std::pair<int, int>> edges;
    backtrack((1u << k) - 1, terminals[0], edges);
    for (auto i : edges) {
        std::cout << i.first + 1 << " " << i.second + 1 << std::endl;
    }
    for (auto i : graph.getPreselectedEdges()) {
        std::cout << i.first + 1 << " " << i.second + 1 << std::endl;
    }

    return Graph();
}

void DreyfusWagner::backtrack(unsigned subset, int root, std::vector<std::pair<int, int>> &tree) {
    if (__builtin_popcount(subset) == 1) {
        memset(closed, 0, sizeof(int) * graph.getNodeCount());
        std::priority_queue<std::pair<unsigned, unsigned>, std::vector<std::pair<unsigned, unsigned>>,
                std::greater<std::pair<unsigned, unsigned>>> dijkstra_q;
        for (int i = 0; i < graph.getNodeCount(); i++) {
            dist[i] = INFTY;
            parent[i] = -1;
        }

        int from = graph.getTerminals()[__builtin_ffs(subset) - 1];
        dist[from] = 0;
        dijkstra_q.push({dist[from], from});
        while (dijkstra_q.size()) {
            std::pair<int, int> curr = dijkstra_q.top();
            dijkstra_q.pop();
            if (closed[curr.second]) continue;
            closed[curr.second] = 1;
            for (auto adj : graph.getAdjacentOf(curr.second)) {
                if (closed[adj.first]) continue;
                if (dist[adj.first] > (unsigned)curr.first + adj.second) {
                    dist[adj.first] = (unsigned)curr.first + adj.second;
                    parent[adj.first] = curr.second;
                    dijkstra_q.push({dist[adj.first], adj.first});
                }
            }
        }

        int curr = root, prev = parent[curr];
        while (prev != -1) {
            tree.push_back({curr, prev});
            curr = prev;
            prev = parent[curr];
        }
        return;
    }

    memset(closed, 0, sizeof(int) * graph.getNodeCount());
    std::priority_queue<std::pair<unsigned, unsigned>, std::vector<std::pair<unsigned, unsigned>>,
            std::greater<std::pair<unsigned, unsigned>>> dijkstra_q;
    for (int i = 0; i < graph.getNodeCount(); i++) {
        if (graph.isNodeErased(i)) {
            continue;
        }
        dist[i] = dp[dp_par[subset][i]][i]
                  + dp[subset - dp_par[subset][i]][i];
        parent[i] = -1;
        dijkstra_q.push({dist[i], i});
    }
    while (dijkstra_q.size()) {
        std::pair<int, int> curr = dijkstra_q.top();
        dijkstra_q.pop();
        if (closed[curr.second]) continue;
        closed[curr.second] = 1;
        for (auto adj : graph.getAdjacentOf(curr.second)) {
            if (closed[adj.first]) continue;
            if (dist[adj.first] > (unsigned)curr.first + adj.second) {
                dist[adj.first] = (unsigned)curr.first + adj.second;
                parent[adj.first] = curr.second;
                dijkstra_q.push({dist[adj.first], adj.first});
            }
        }
    }

    int curr = root, prev = parent[curr];
    while (prev != -1) {
        tree.push_back({curr, prev});
        curr = prev;
        prev = parent[curr];
    }

    backtrack(dp_par[subset][curr], curr, tree);
    backtrack(subset - dp_par[subset][curr], curr, tree);
}
