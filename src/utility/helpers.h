#ifndef PACE2018_HELPERS_H
#define PACE2018_HELPERS_H

#include <algorithm>
#include <cstdint>
#include <map>
#include <vector>

/**
 * Divides two vectors into intersection, and exclusives
 */
template <typename T>
void divide(std::vector<T> &setA, std::vector<T> &setB, std::vector<T> &intersection,
            std::vector<T> &exclusiveA, std::vector<T> &exclusiveB, bool sorted = false) {
    if (!sorted) {
        std::sort(setA.begin(), setA.end());
        std::sort(setB.begin(), setB.end());
    }
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

inline std::vector<char> partitionToVec(int size, uint64_t partition) {
    std::vector<char> vec;
    for (int i = 0; i < size; ++i) {
        uint64_t value = (partition & (0xFLL << (i << 2))) >> (i << 2);
        vec.push_back((char)value);
    }
    return vec;
}

inline int getComponentAt(uint64_t partition, int at) {
    uint64_t comp = (partition & (0xFLL << (at << 2))) >> (at << 2);
    return (int)comp;
}

inline uint64_t vecToPartition(const std::vector<char> &vec, int subset) {
    // canonize the vector
    std::map<char, char> partMap;
    char counter = 0, idx = 0;
    for (auto i : vec) {
        if (!(subset & (1 << idx))) {
            partMap[i] = 0;
        } else if (!partMap.count(i)) {
            partMap[i] = counter++;
        }
        idx++;
    }

    // translate to integer type
    uint64_t part = 0;
    int shift = 0;
    for (char i : vec) {
        part |= ((uint64_t)partMap[i]) << shift;
        shift += 4;
    }
    return part;
}

inline uint64_t partitionWithoutElement(const std::vector<char> &vec, int id, int subset) {
    std::vector<char> newVec = vec;
    newVec.erase(newVec.begin() + id);
    return vecToPartition(newVec, subset);
}

inline int maskWithoutElement(int mask, int id, int size) {
    int shift = 0, result = 0;
    for (int i = 0; i < size; i++) {
        if (i == id) continue;
        result |= (((mask & (1 << i)) != 0)) << shift;
        shift++;
    }
    return result;
}

inline int maskWithElement(int mask, int id, int value, int size) {
    int shift = 0, result = 0;
    for (int i = 0; i < size; i++) {
        if (i == id) {
            result |= (value << shift);
            shift++;
        }
        result |= ((mask & (1 << i)) != 0) << shift;
        shift++;
    }
    if (size == id) {
        result |= (value << shift);
    }
    return result;
}

inline uint64_t cyclicMerge(uint64_t part1, uint64_t part2) {
    // TODO
}

#endif //PACE2018_HELPERS_H
