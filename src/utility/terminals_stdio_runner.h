#ifndef PACE2018_TERMINALS_STDIO_RUNNER_H
#define PACE2018_TERMINALS_STDIO_RUNNER_H

#include <iostream>
#include <memory>

#include "solvers/base_dp_solver.h"
#include "solvers/dreyfus_wagner.h"
#include "solvers/reduce_dp_solver.h"
#include "solvers/table_dp_solver.h"
#include "structures/graph.h"
#include "structures/tree_decomposition.h"

class TerminalsStdioRunner {
public:
    void run();
};


#endif //PACE2018_TERMINALS_STDIO_RUNNER_H
