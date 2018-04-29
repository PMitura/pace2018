#include "utility/stdio_runner.h"

int sum(int a, int b) {
    return a + b;
}

int main() {
    StdioRunner runner;
    runner.run();
    return 0;
}