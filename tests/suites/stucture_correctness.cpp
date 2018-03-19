#include <gtest/gtest.h>
#include <fstream>
#include <map>

#include "structures/graph.h"
#include "structures/tree_decomposition.h"

TEST(Structures, GraphSimple) {
    Graph g;
    std::ifstream simple1("tests/inputs/simple1.gr");
    g.load(simple1);

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

    std::vector<int> bag0 = {0, 1, 2};
    std::vector<int> bag1 = {1, 2, 3};
    std::vector<int> bag2 = {2, 3, 4};
    std::vector<int> bag3 = {};

    EXPECT_EQ(td.getBagOf(0), bag0);
    EXPECT_EQ(td.getBagOf(1), bag1);
    EXPECT_EQ(td.getBagOf(2), bag2);
    EXPECT_EQ(td.getBagOf(3), bag3);

    std::vector<int> adj0 = {1};
    std::vector<int> adj1 = {0, 2, 3};
    std::vector<int> adj2 = {1};
    std::vector<int> adj3 = {1};

    EXPECT_EQ(td.getAdjacentTo(0), adj0);
    EXPECT_EQ(td.getAdjacentTo(1), adj1);
    EXPECT_EQ(td.getAdjacentTo(2), adj2);
    EXPECT_EQ(td.getAdjacentTo(3), adj3);
}
