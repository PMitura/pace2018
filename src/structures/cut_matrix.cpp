#include "cut_matrix.h"

void CutMatrix::generate(const std::vector<uint64_t> &sortedPartitions,
                         unsigned subset, unsigned size) {
    generateCuts(subset, size);
    for (auto part : sortedPartitions) {
        transformToRow(part, subset, size);
    }
}

void CutMatrix::eliminate() {
    std::vector<Row> basis;

    for (unsigned scan = 0; scan < rows.size(); scan++) {
        int scanLSB = rows[scan].lsb();
        // zero row
        if (scanLSB == -1) {
            continue;
        }
        basis.push_back(rows[scan]);
        for (unsigned elim = scan + 1; elim < rows.size(); elim++) {
            if (rows[elim].at((unsigned)scanLSB)) {
                rows[elim].add(rows[scan]);
            }
        }
    }

    rows = basis;
}

std::vector<uint64_t> CutMatrix::getPartitions() const {
    std::vector<uint64_t> partitions;
    for (auto row : rows) {
        partitions.push_back(row.partition);
    }
    return partitions;
}

void CutMatrix::generateCuts(unsigned subset, unsigned size) {
    unsigned subsetSize = __builtin_popcount(subset);

    for (unsigned cutSubset = 0; cutSubset < (1u << (subsetSize-1)); cutSubset++) {
        // scatter bits by mask
        unsigned cut = 0, ptr = 0;
        for (unsigned i = 0; i < size; i++) {
            // skip unused node
            if (!isInSubset(i, subset)) {
                continue;
            }
            // set bit if it's present in the subset cut
            if ((cutSubset & (1u << ptr++)) != 0) {
                cut |= (1u << i);
            }
        }
        cuts.push_back(cut);
    }
}

void CutMatrix::transformToRow(uint64_t partition, unsigned subset, unsigned size) {
    // set size of bitset to ceil(cuts / 64) 64bit integers
    Row row;
    row.bset.resize(((cuts.size() + 63u) >> 6u));
    row.partition = partition;

    for (unsigned cutId = 0; cutId < (unsigned)cuts.size(); cutId++) {
        if (partitionRefinesCut(partition, cuts[cutId], subset, size)) {
            row.bset[cutId >> 6u] |= 1ull << (cutId % 64);
        }
    }
    rows.push_back(row);
}

bool CutMatrix::partitionRefinesCut(uint64_t partition, unsigned cut,
                                    unsigned subset, unsigned size) {
    unsigned isInZero = 0, isInOne = 0;
    for (unsigned i = 0; i < size; i++) {
        if (!isInSubset(i, subset)) {
            continue;
        }
        if (isInSubset(i, cut)) {
            isInOne |= 1u << (unsigned)getComponentAt(partition, i);
        } else {
            isInZero |= 1u << (unsigned)getComponentAt(partition, i);
        }
    }
    return (isInOne & isInZero) == 0;
}
