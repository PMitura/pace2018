#include <gtest/gtest.h>

#include "utility/partition_mergers.h"

TEST(Mergers, VectorDFSMerger) {
    uint64_t a1   = 0x2110,
             b1   = 0x2210,
             ref1 = 0x1110;
    VectorDFSMerger merger1(ref1, 4, 0b1111);
    EXPECT_EQ(ref1, merger1.merge(a1, b1));

    uint64_t a2   = 0x2100,
             b2   = 0x0110,
             ref2 = 0x0000;
    VectorDFSMerger merger2(ref2, 4, 0b1111);
    EXPECT_EQ(ref2, merger2.merge(a2, b2));

    uint64_t a3 = 0x221100,
             b3 = 0x222110,
             ref3 = 0x0;
    VectorDFSMerger merger3(ref3, 6, 0b111111);
    EXPECT_EQ(PARTITION_INVALID, merger3.merge(a3, b3));
}

TEST(Mergers, BinaryDFSMerger) {
    uint64_t a1   = 0x2110,
            b1   = 0x2210,
            ref1 = 0x1110;
    BinaryDFSMerger merger1(ref1, 4, 0b1111);
    EXPECT_EQ(ref1, merger1.merge(a1, b1));

    uint64_t a2   = 0x2100,
            b2   = 0x0110,
            ref2 = 0x0000;
    BinaryDFSMerger merger2(ref2, 4, 0b1111);
    EXPECT_EQ(ref2, merger2.merge(a2, b2));

    uint64_t a3 = 0x221100,
            b3 = 0x222110,
            ref3 = 0x0;
    BinaryDFSMerger merger3(ref3, 6, 0b111111);
    EXPECT_EQ(PARTITION_INVALID, merger3.merge(a3, b3));
}

TEST(Mergers, UnionFindMerger) {
    uint64_t a1   = 0x2110,
            b1   = 0x2210,
            ref1 = 0x1110;
    UnionFindMerger merger1(ref1, 4, 0b1111);
    EXPECT_EQ(ref1, merger1.merge(a1, b1));

    uint64_t a2   = 0x2100,
            b2   = 0x0110,
            ref2 = 0x0000;
    UnionFindMerger merger2(ref2, 4, 0b1111);
    EXPECT_EQ(ref2, merger2.merge(a2, b2));

    uint64_t a3 = 0x221100,
            b3 = 0x222110,
            ref3 = 0x0;
    UnionFindMerger merger3(ref3, 6, 0b111111);
    EXPECT_EQ(PARTITION_INVALID, merger3.merge(a3, b3));
}
