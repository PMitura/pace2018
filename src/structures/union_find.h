#ifndef PACE2018_UNION_FIND_H
#define PACE2018_UNION_FIND_H

#include <vector>

class UnionFind {
public:
    explicit UnionFind(unsigned cnt) : cnt(cnt) {
        next.resize(cnt);
        size.resize(cnt);
    }

    void setupPairs() {
        for (unsigned i = 0; i < (cnt >> 1u); i++) {
            next[i] = (char)i;
            size[i] = 2;
        }
        for (unsigned i = (cnt >> 1u); i < cnt; i++) {
            next[i] = (char)(i - (cnt >> 1u));
            size[i] = 2;
        }
    }

    char find(char x) {
        while (x != next[x]) {
            x = next[x];
        }
        return x;
    }

    void join(char x, char y) {
        char xc = find(x), yc = find(y);
        if (size[xc] < size[yc]) {
            size[yc] += size[xc];
            next[xc] = yc;
        } else {
            size[xc] += size[yc];
            next[yc] = xc;
        }
    }

private:
    unsigned cnt;
    std::vector<char> next, size;
};


#endif //PACE2018_UNION_FIND_H
