#ifndef PACE2018_PARTITION_MERGERS_H
#define PACE2018_PARTITION_MERGERS_H

#include <cstdint>
#include <vector>

#include "utility/helpers.h"

const uint64_t PARTITION_INVALID = 0xFFFFFFFFFFFFFFFF;

class PartitionMerger {
public:
    PartitionMerger(uint64_t mergedPart, int size, unsigned subset)
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

#endif //PACE2018_PARTITION_MERGERS_H
