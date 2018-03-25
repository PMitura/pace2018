#include <gtest/gtest.h>

#include "utility/partitioner.h"

TEST(Partitioner, Basic) {
    Partitioner part1(0, 0b111, 3);
    part1.compute();
    std::vector<uint64_t> vres1 = part1.getResult();
    std::set<uint64_t> ref1 = {0x0000, 0x0100, 0x0010, 0x0110, 0x0210},
                       res1(vres1.begin(), vres1.end());
    EXPECT_EQ(ref1, res1);
}
