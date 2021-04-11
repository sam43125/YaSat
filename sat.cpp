#include <iostream>
#include <vector>

#undef NDEBUG
#include <cassert>

#include "parser.h"
#include "solver.hpp"

typedef std::vector<int>::iterator vIter;
typedef std::vector<int> clause_t;

int main(int argc, char **argv) {

    assert("Usage: ./yasat [input.cnf]" && argc > 1);

    std::vector<clause_t> clauses;
    int maxVarIndex;

    parse_DIMACS_CNF(clauses, maxVarIndex, argv[1]);

    Solver solver(clauses, maxVarIndex);

    return 0;
}