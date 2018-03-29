#ifndef PACE2018_PARTITION_MERGERS_H
#define PACE2018_PARTITION_MERGERS_H

#include <cstdint>
#include <vector>
#include <set>

#include "utility/helpers.h"

const uint64_t PARTITION_INVALID = 0xFFFFFFFFFFFFFFFF;

class PartitionMerger {
public:
    PartitionMerger(uint64_t mergedPart, unsigned size, unsigned subset)
            : mergedPart(mergedPart), size(size), subset(subset) {}

    virtual uint64_t merge(uint64_t part1, uint64_t part2) = 0;

protected:
    uint64_t mergedPart;
    unsigned size, subset;
};


class VectorDFSMerger : public PartitionMerger {
public:
    VectorDFSMerger(uint64_t mergedPart, unsigned size, unsigned subset)
            : PartitionMerger(mergedPart, size, subset) {
        vpartMerged = partitionToVec(size, mergedPart);
    }

    uint64_t merge(uint64_t part1, uint64_t part2) override ;

private:
    bool dfs(char color, unsigned idx);

    std::vector<char> colors, vpart1, vpart2, vpartMerged;
};

class BinaryDFSMerger : public PartitionMerger {
public:
    BinaryDFSMerger(uint64_t mergedPart, unsigned size, unsigned subset)
            : PartitionMerger(mergedPart, size, subset),
              part1(0), part2(0) {
        mergedCompCnt = maxComponentIn(mergedPart, size) + 1;
    }

    uint64_t merge(uint64_t part1, uint64_t part2) override ;

private:
    bool dfs(char color, unsigned idx);

    uint64_t part1, part2;
    int mergedCompCnt;
    std::vector<char> colors;
};

#endif //PACE2018_PARTITION_MERGERS_H
