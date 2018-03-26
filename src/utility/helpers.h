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

inline int isInSubset(int idx, int subset) {
    return subset & (1 << idx);
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
        if (!(subset & (1 << idx++))) {
            continue;
        } else if (!partMap.count(i)) {
            partMap[i] = counter++;
        }
    }

    // translate to integer type
    uint64_t part = 0;
    int shift = 0;
    idx = 0;
    for (char i : vec) {
        if (subset & (1 << idx++)) {
            part |= ((uint64_t) partMap[i]) << shift;
        }
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

inline bool mergeDFS(std::vector<char> &colors, char usedColor,
                    std::vector<char> &part1, std::vector<char> &part2,
                    int idx, int size, int subset) {
    if (!(subset & (1 << idx))) return false;
    if (colors[idx] != -1) return false;
    colors[idx] = usedColor;
    for (int i = 0; i < size; i++) {
        if (idx == i) continue;
        if (part1[i] == part1[idx] || part2[i] == part2[idx]) {
            mergeDFS(colors, usedColor, part1, part2, i, size, subset);
        }
    }
    return true;
}

const uint64_t INVALID = 0xFFFFFFFFFFFFFFFF;

inline uint64_t acyclicMerge(uint64_t part1, uint64_t part2, uint64_t mainpart, int size, int subset) {
    std::vector<char> vpart1 = partitionToVec(size, part1),
                      vpart2 = partitionToVec(size, part2),
                      vpart3 = partitionToVec(size, mainpart);

    // cycle/sanity check
    int parcnt1 = (*std::max_element(vpart1.begin(), vpart1.end())) + 1,
        parcnt2 = (*std::max_element(vpart2.begin(), vpart2.end())) + 1,
        parcnt3 = (*std::max_element(vpart3.begin(), vpart3.end())) + 1;
    if (__builtin_popcount(subset) != parcnt1 + parcnt2 - parcnt3) {
        return INVALID;
    }

    std::vector<char> colors;
    colors.resize((size_t)size, -1);
    char currColor = 0;
    for (int i = 0; i < size; i++) {
        if (mergeDFS(colors, currColor, vpart1, vpart2, i, size, subset)) {
            currColor++;
        }
    }

    return vecToPartition(colors, subset);
}

inline uint64_t cyclicMerge(uint64_t part1, uint64_t part2, int size, int subset) {
    std::vector<char> vpart1 = partitionToVec(size, part1),
                      vpart2 = partitionToVec(size, part2);
    std::vector<char> colors;
    colors.resize((size_t)size, -1);
    char currColor = 0;
    for (int i = 0; i < size; i++) {
        if (mergeDFS(colors, currColor, vpart1, vpart2, i, size, subset)) {
            currColor++;
        }
    }

    return vecToPartition(colors, subset);
}

#endif //PACE2018_HELPERS_H
