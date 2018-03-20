#include <gtest/gtest.h>
#include <set>
#include <vector>

#include "utility/helpers.h"

TEST(Helpers, Divide) {
    std::vector<int> setA = {4, 2, 9, 3, 5},
                     setB = {9, 6, 0, 5, 1};
    std::set<int> refExclusiveA = {2, 3, 4},
                  refExclusiveB = {0, 1, 6},
                  refIntersect  = {5, 9};

    std::vector<int> exA, exB, inter;
    divide(setA, setB, inter, exA, exB);
    std::set<int> setExclusiveA(exA.begin(), exA.end());
    std::set<int> setExclusiveB(exB.begin(), exB.end());
    std::set<int> setIntersect(inter.begin(), inter.end());

    EXPECT_EQ(setExclusiveA, refExclusiveA);
    EXPECT_EQ(setExclusiveB, refExclusiveB);
    EXPECT_EQ(setIntersect,  refIntersect);
}
