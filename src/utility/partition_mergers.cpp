#include "partition_mergers.h"

uint64_t VectorDFSMerger::merge(uint64_t part1, uint64_t part2) {
    vpart1 = partitionToVec(size, part1);
    vpart2 = partitionToVec(size, part2);

    // cycle/sanity check
    int parcnt1 = (*std::max_element(vpart1.begin(), vpart1.end())) + 1,
        parcnt2 = (*std::max_element(vpart2.begin(), vpart2.end())) + 1,
        parcnt3 = (*std::max_element(vpartMerged.begin(), vpartMerged.end())) + 1;
    if (__builtin_popcount(subset) != parcnt1 + parcnt2 - parcnt3) {
        return PARTITION_INVALID;
    }

    colors.clear();
    colors.resize((size_t)size, -1);
    char currColor = 0;
    for (unsigned i = 0; i < size; i++) {
        if (dfs(currColor, i)) {
            currColor++;
        }
    }

    return vecToPartition(colors, subset);
}

bool VectorDFSMerger::dfs(char color, unsigned idx) {
    if ((subset & (1u << idx)) == 0) {
        return false;
    }
    if (colors[idx] != -1) {
        return false;
    }
    colors[idx] = color;
    for (unsigned i = 0; i < size; i++) {
        if (idx == i) {
            continue;
        }
        if (vpart1[i] == vpart1[idx] || vpart2[i] == vpart2[idx]) {
            dfs(color, i);
        }
    }
    return true;
}

uint64_t BinaryDFSMerger::merge(uint64_t part1, uint64_t part2) {
    this->part1 = part1;
    this->part2 = part2;

    // cycle/sanity check
    int parcnt1 = maxComponentIn(part1, size) + 1,
        parcnt2 = maxComponentIn(part2, size) + 1;
    if (__builtin_popcount(subset) != parcnt1 + parcnt2 - mergedCompCnt) {
        return PARTITION_INVALID;
    }

    colors.clear();
    colors.resize((size_t)size, -1);
    char currColor = 0;
    for (unsigned i = 0; i < size; i++) {
        // serves as bound for searching adjacent nodes
        if (dfs(currColor, i)) {
            currColor++;
        }
    }

    uint64_t result = 0;
    for (unsigned i = 0; i < size; i++) {
        if (isInSubset(i, subset)) {
            result |= (uint64_t) colors[i] << (i << 2llu);
        }
    }
    return result;
}

bool BinaryDFSMerger::dfs(char color, unsigned idx) {
    if ((subset & (1u << idx)) == 0) {
        return false;
    }
    if (colors[idx] != -1) {
        return false;
    }
    colors[idx] = color;
    for (unsigned i = 0; i < size; i++) {
        if (idx == i) {
            continue;
        }
        if (   getComponentAt(part1, i) == getComponentAt(part1, idx)
            || getComponentAt(part2, i) == getComponentAt(part2, idx)) {
            dfs(color, i);
        }
    }
    return true;
}