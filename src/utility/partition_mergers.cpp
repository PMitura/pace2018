#include <iostream>
#include "partition_mergers.h"

uint64_t VectorDFSMerger::merge(uint64_t part1, uint64_t part2) {
    vpart1 = partitionToVec(size, part1);
    vpart2 = partitionToVec(size, part2);

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

    std::fill(colors.begin(), colors.end(), -1);
    char currColor = 0;
    earlyStop = false;
    for (unsigned i = 0; i < size; i++) {
        // serves as bound for searching adjacent nodes
        if (dfs(currColor, i)) {
            currColor++;
        }
        if (earlyStop) {
            return PARTITION_INVALID;
        }
    }
    return mergedPart;
}

bool BinaryDFSMerger::dfs(char color, unsigned idx) {
    if ((subset & (1u << idx)) == 0) {
        return false;
    }
    if (colors[idx] != -1) {
        return false;
    }
    // not the intended color
    if (color != getComponentAt(mergedPart, idx)) {
        earlyStop = true;
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
            if (earlyStop) {
                return false;
            }
        }
    }
    return true;
}

uint64_t UnionFindMerger::merge(uint64_t part1, uint64_t part2) {
    unionFind.setupPairs();
    std::fill(repre1.begin(), repre1.end(), -1);
    std::fill(repre2.begin(), repre2.end(), -1);
    for (unsigned char i = 0; i < size; i++) {
        if (!isInSubset(i, subset)) {
            continue;
        }

        int comp1 = getComponentAt(part1, i),
            comp2 = getComponentAt(part2, i);

        if (repre1[comp1] == -1) {
            repre1[comp1] = i;
        } else {
            if (!unionFind.join(i, repre1[comp1])) {
                // cyclic merge
                return PARTITION_INVALID;
            }
        }
        if (repre2[comp2] == -1) {
            repre2[comp2] = i;
        } else {
            if (!unionFind.join((char)size + i, (char)size + repre2[comp2])) {
                // cyclic merge
                return PARTITION_INVALID;
            }
        }
    }

    uint64_t result = 0, ctr = 0;
    std::fill(mapping.begin(), mapping.end(), -1);
    for (unsigned i = 0; i < size; i++) {
        if (!isInSubset(i, subset)) {
            continue;
        }

        int comp = unionFind.find((char)i);
        if (mapping[comp] == -1) {
            mapping[comp] = (char)ctr++;
        }
        result |= (uint64_t)mapping[comp] << (i << 2ull);
    }

    return result;
}
