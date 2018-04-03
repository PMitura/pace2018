#ifndef PACE2018_HELPERS_H
#define PACE2018_HELPERS_H

#include <algorithm>
#include <cstdint>
#include <map>
#include <stack>
#include <vector>

/**
 * Divides two vectors into intersection, and exclusives
 */
template <typename T>
void divide(std::vector<T> &setA, std::vector<T> &setB, std::vector<T> &intersection,
            std::vector<T> &exclusiveA, std::vector<T> &exclusiveB) {
    std::sort(setA.begin(), setA.end());
    std::sort(setB.begin(), setB.end());
    std::set_intersection(setA.begin(), setA.end(),
                          setB.begin(), setB.end(),
                          std::back_inserter(intersection));
    std::set_difference(setA.begin(), setA.end(),
                        setB.begin(), setB.end(),
                        std::back_inserter(exclusiveA));
    std::set_difference(setB.begin(), setB.end(),
                        setA.begin(), setA.end(),
                        std::back_inserter(exclusiveB));
}

inline bool isInSubset(unsigned idx, unsigned subset) {
    return (bool)((subset & (1u << idx)) != 0);
}

inline std::vector<char> partitionToVec(unsigned size, uint64_t partition) {
    std::vector<char> vec;
    for (unsigned i = 0; i < size; ++i) {
        uint64_t value = (partition & (0xFull << (i << 2u))) >> (i << 2u);
        vec.push_back((char)value);
    }
    return vec;
}

inline int getComponentAt(uint64_t partition, unsigned at) {
    uint64_t comp = (partition & (0xFULL << (at << 2u))) >> (at << 2u);
    return (int)comp;
}

inline int maxComponentIn(uint64_t partition, unsigned size) {
    int result = 0;
    for (unsigned i = 0; i < size; ++i) {
        result = std::max(result, getComponentAt(partition, i));
    }
    return result;
}

inline uint64_t vecToPartition(const std::vector<char> &vec, unsigned subset) {
    // canonize the vector
    std::map<char, char> partMap;
    unsigned counter = 0, idx = 0;
    for (auto i : vec) {
        if ((subset & (1u << idx++)) == 0) {
            continue;
        } else if (partMap.count(i) == 0) {
            partMap[i] = (char)counter++;
        }
    }

    // translate to integer type
    uint64_t part = 0;
    unsigned shift = 0;
    idx = 0;
    for (char i : vec) {
        if ((subset & (1u << idx++)) != 0) {
            part |= ((uint64_t) partMap[i]) << shift;
        }
        shift += 4;
    }
    return part;
}

inline uint64_t partitionWithoutElement(const std::vector<char> &vec, int id, unsigned subset) {
    std::vector<char> newVec = vec;
    newVec.erase(newVec.begin() + id);
    return vecToPartition(newVec, subset);
}

inline unsigned maskWithoutElement(unsigned mask, unsigned id, unsigned size) {
    unsigned shift = 0, result = 0;
    for (unsigned i = 0; i < size; i++) {
        if (i == id) {
            continue;
        }
        result |= (unsigned)((mask & (1u << i)) != 0) << shift;
        shift++;
    }
    return result;
}

inline unsigned maskWithElement(unsigned mask, unsigned id, unsigned value, unsigned size) {
    unsigned shift = 0, result = 0;
    for (unsigned i = 0; i < size; i++) {
        if (i == id) {
            result |= (value << shift);
            shift++;
        }
        result |= (unsigned)((mask & (1u << i)) != 0) << shift;
        shift++;
    }
    if (size == id) {
        result |= (value << shift);
    }
    return result;
}

#endif //PACE2018_HELPERS_H
