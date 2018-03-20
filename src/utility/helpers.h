#ifndef PACE2018_HELPERS_H
#define PACE2018_HELPERS_H

#include <algorithm>
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

#endif //PACE2018_HELPERS_H
