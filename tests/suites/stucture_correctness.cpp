#include <gtest/gtest.h>
#include <fstream>
#include <map>

#include "structures/graph.h"
#include "structures/tree_decomposition.h"

TEST(Structures, GraphSimple) {
    Graph g;
    std::ifstream simple1("tests/inputs/simple1.gr");
    g.load(simple1);

    EXPECT_EQ(g.getNodes(), 5);
    EXPECT_EQ(g.getEdges(), 6);
    EXPECT_EQ(g.getTermCount(), 2);

    EXPECT_TRUE(g.isTerm(1));
    EXPECT_TRUE(g.isTerm(3));
    EXPECT_FALSE(g.isTerm(0));
    EXPECT_FALSE(g.isTerm(2));
    EXPECT_FALSE(g.isTerm(4));

    std::map<int, int> edgesOf0 = {{1, 1}, {3, 3}};
    EXPECT_EQ(g.getAdjacentOf(0), edgesOf0);

    std::map<int, int> edgesOf2 = {{1, 3}, {4, 10}};
    EXPECT_EQ(g.getAdjacentOf(2), edgesOf2);
}

TEST(Structures, TreeDecompositionSimple) {
    TreeDecomposition td;
    std::ifstream simple1("tests/inputs/simple1.td");
    td.load(simple1);
}
