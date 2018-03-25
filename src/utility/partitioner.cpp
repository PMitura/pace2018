#include "partitioner.h"

void Partitioner::compute() {
    vPartition = partitionToVec(size, partition);
    compMapping.clear();
    result.clear();

    std::vector<char> generated;
    generateRec(0, 0, generated);
}

void Partitioner::generateRec(int idx, char newPart, std::vector<char> &generated) {
    // partition generated
    if (idx == size) {
        result.push_back(vecToPartition(generated, subset));
        return;
    }

    // not in subset
    if (!(subset & (1 << idx))) {
        generated.push_back(0);
        generateRec(idx+1, newPart, generated);
        generated.pop_back();
        return;
    }

    // place in existing component of this class
    for (auto comp : compMapping[vPartition[idx]]) {
        generated.push_back(comp);
        generateRec(idx+1, newPart, generated);
        generated.pop_back();
    }

    // new component
    generated.push_back(newPart);
    compMapping[vPartition[idx]].push_back(newPart);
    generateRec(idx+1, newPart+(char)1, generated);
    compMapping[vPartition[idx]].pop_back();
    generated.pop_back();
}

const std::vector<uint64_t> &Partitioner::getResult() const {
    return result;
}
