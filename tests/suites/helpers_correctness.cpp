#include <gtest/gtest.h>

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

    EXPECT_EQ(refExclusiveA, setExclusiveA);
    EXPECT_EQ(refExclusiveB, setExclusiveB);
    EXPECT_EQ(refIntersect,  setIntersect);
}

TEST(Helpers, PartitionToVec) {
    std::vector<char> refVec1 = {3, 8, 0, 15};
    uint64_t partition1 = 0x000000000000F083;
    EXPECT_EQ(refVec1, partitionToVec(4, partition1));

    std::vector<char> refVec2 = {0, 1, 2, 3,
                                 4, 5, 6, 1,
                                 7, 8, 2, 9,
                                 10, 5, 7, 2};
    uint64_t partition2 = 0x275a928716543210;
    EXPECT_EQ(refVec2, partitionToVec(16, partition2));

    std::vector<char> refVec3 = {};
    uint64_t partition3 = 0xDEADBEEF;
    EXPECT_EQ(refVec3, partitionToVec(0, partition3));
}

::testing::AssertionResult assertHexaValues(const char * xExpr,
                                            const char * yExpr,
                                            uint64_t x, uint64_t y) {
    if (x == y) {
        return ::testing::AssertionSuccess();
    }

    // get hexa representation to strings
    // reason: https://github.com/google/googletest/issues/779
    std::stringstream hexX, hexY;
    hexX << std::hex << x;
    hexY << std::hex << y;
    std::string strhexX, strhexY;
    hexX >> strhexX;
    hexY >> strhexY;

    return ::testing::AssertionFailure() << xExpr << " and " << yExpr << " mismatch"
                                         << std::endl
                                         << "Result:   " << strhexX
                                         << std::endl
                                         << "Expected: " << strhexY;
}

TEST(Helpers, VecToPartition) {
    int subset = 0xFFFF;

    std::vector<char> vec1 = {3, 8, 0, 15};
    uint64_t partition1 = 0x0000000000003210;
    EXPECT_PRED_FORMAT2(assertHexaValues, vecToPartition(vec1, subset), partition1);

    std::vector<char> vec2 = {14, 3, 2, 1,
                              4, 8, 10, 3,
                              0, 5, 2, 9,
                              15, 8, 0, 2};
    uint64_t partition2 = 0x275a928716543210;
    EXPECT_PRED_FORMAT2(assertHexaValues, vecToPartition(vec2, subset), partition2);

    std::vector<char> vec3 = {};
    uint64_t partition3 = 0;
    EXPECT_PRED_FORMAT2(assertHexaValues, vecToPartition(vec3, subset), partition3);

    // mask test
    int subset2 = 0b1101, subset3 = 0b1011;
    std::vector<char> vec4 = {2, 5, 7, 9};
    uint64_t partition4_2 = 0x2100,
             partition4_3 = 0x2010;
    EXPECT_PRED_FORMAT2(assertHexaValues, vecToPartition(vec4, subset2), partition4_2);
    EXPECT_PRED_FORMAT2(assertHexaValues, vecToPartition(vec4, subset3), partition4_3);
}

TEST(Helpers, MaskWithoutElement) {
    int mask1 = 0b00001010,
        ref1  = 0b00000110;
    EXPECT_EQ(ref1, maskWithoutElement(mask1, 2, 4));

    int mask2 = 0b00011001,
        ref2  = 0b00001100;
    EXPECT_EQ(ref2, maskWithoutElement(mask2, 0, 8));
}

TEST(Helpers, MaskWithElement) {
    int mask1 = 0b00001010,
        ref1  = 0b00011010;
    EXPECT_EQ(ref1, maskWithElement(mask1, 3, 1, 4));

    int mask2 = 0b00001010,
        ref2  = 0b00010010;
    EXPECT_EQ(ref2, maskWithElement(mask2, 3, 0, 4));

    int mask3 = 0b00011101,
        ref3  = 0b00111011;
    EXPECT_EQ(ref3, maskWithElement(mask3, 0, 1, 5));
}

TEST(Helpers, GetComponentAt) {
    std::vector<char> refVec2Canon = {0, 1, 2, 3,
                                      4, 5, 6, 1,
                                      7, 8, 2, 9,
                                      10, 5, 7, 2};
    uint64_t partition2 = 0x275a928716543210;
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(refVec2Canon[i], getComponentAt(partition2, i));
    }
}