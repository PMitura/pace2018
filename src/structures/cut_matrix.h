#ifndef PACE2018_CUT_MATRIX_H
#define PACE2018_CUT_MATRIX_H

#include <cstdint>
#include <iostream>
#include <vector>
#include <unordered_map>

#include "utility/helpers.h"

class CutMatrix {
public:
    void generate(const std::vector<uint64_t>& sortedPartitions,
                  unsigned subset, unsigned size);

    void eliminate();

    std::vector<uint64_t> getPartitions() const;

private:
    struct Row {
        std::vector<uint64_t> bset;
        uint64_t partition;

        Row() : partition(0) {}

        int lsb() {
            int mul = 0;
            for (auto i : bset) {
                if (i) {
                     return __builtin_ffsll(i) - 1 + (mul << 8u);
                }
                mul++;
            }
            return -1;
        };

        void add(const Row& r) {
            for (unsigned i = 0; i < bset.size(); i++) {
                bset[i] ^= r.bset[i];
            }
        }
    };

    void generateCuts(unsigned subset, unsigned size);

    void transformToRow(uint64_t partition, unsigned subset, unsigned size);

    bool partitionRefinesCut(uint64_t partition, unsigned cut,
                             unsigned subset, unsigned size);

    std::vector<unsigned> cuts;
    std::vector<Row> rows;
};


#endif //PACE2018_CUT_MATRIX_H
