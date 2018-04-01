#ifndef PACE2018_UNION_FIND_H
#define PACE2018_UNION_FIND_H

#include <vector>

class UnionFind {
public:
    explicit UnionFind(unsigned cnt) {
        next.resize(2*cnt);
        size.resize(2*cnt);
        for (unsigned i = 0; i < cnt; i++) {
            next[i] = (char)i;
            size[i] = 2;
        }
        for (unsigned i = cnt; i < 2*cnt; i++) {
            next[i] = (char)(i - cnt);
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
    std::vector<char> next, size;
};


#endif //PACE2018_UNION_FIND_H
