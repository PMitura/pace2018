#ifndef PACE2018_PARTITION_MERGERS_H
#define PACE2018_PARTITION_MERGERS_H

#include <cstdint>
#include <cstring>
#include <vector>
#include <set>

#include "structures/union_find.h"
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
              part1(0), part2(0), earlyStop(false) {
        colors.resize(size);
    }

    uint64_t merge(uint64_t part1, uint64_t part2) override;

private:
    bool dfs(char color, unsigned idx);

    uint64_t part1, part2;
    std::vector<char> colors;
    bool earlyStop;
};

class UnionFindMerger : public PartitionMerger {
public:
    UnionFindMerger(uint64_t mergedPart, unsigned size, unsigned subset)
            : PartitionMerger(mergedPart, size, subset),
              unionFind(2*size) {
        repre1.resize(size);
        repre2.resize(size);
        mapping.resize(2*size);
    }

    UnionFindMerger(unsigned size, unsigned subset)
            : PartitionMerger(0, size, subset),
              unionFind(2*size) {
        repre1.resize(size);
        repre2.resize(size);
        mapping.resize(2*size);
    }

    uint64_t merge(uint64_t part1, uint64_t part2) override;

private:
    std::vector<char> repre1, repre2, mapping;
    UnionFind unionFind;
};

#endif //PACE2018_PARTITION_MERGERS_H
