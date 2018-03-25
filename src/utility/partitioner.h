#ifndef PACE2018_PARTITIONER_H
#define PACE2018_PARTITIONER_H

#include <cstdint>
#include <map>
#include <vector>

#include "utility/helpers.h"

class Partitioner {
public:
    Partitioner(uint64_t partition, int subset, int size)
            : partition(partition), subset(subset), size(size) {}
    void compute();
    const std::vector<uint64_t> &getResult() const;

private:
    void generateRec(int idx, char newPart, std::vector<char> &generated);

    uint64_t partition;
    int subset, size;
    std::vector<char> vPartition;
    std::vector<uint64_t> result;
    std::map<int, std::vector<char>> compMapping;
};


#endif //PACE2018_PARTITIONER_H
