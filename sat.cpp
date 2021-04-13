#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iterator>

#undef NDEBUG
#include <cassert>

#include "parser.h"
#include "solver.hpp"

typedef std::vector<int> clause_t;

int main(int argc, char **argv) {

    assert("Usage: ./yasat [input.cnf]" && argc > 1);

    std::vector<clause_t> clauses;
    int maxVarIndex;

    parse_DIMACS_CNF(clauses, maxVarIndex, argv[1]);

    Solver solver(clauses, maxVarIndex);

    std::string output_filename(argv[1]);
    output_filename = output_filename.substr(0UL, output_filename.length() - 4UL) + ".sat";
    std::ofstream output_file(output_filename);
    assert("Cannot open the output file" && output_file.is_open());

    if (solver.DPLL()) {
        output_file << "s SATISFIABLE\nv ";
        auto assignments = solver.getAssignments();
        std::copy(assignments.begin(), assignments.end(), 
                  std::ostream_iterator<int>(output_file, " "));
        output_file << "0\n";
    }
    else {
        output_file << "s UNSATISFIABLE\n";
    }

    output_file.close();
    return 0;
}