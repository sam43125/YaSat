// Reference: https://pyeda.readthedocs.io/en/latest/queens.html
#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>
#include <iterator>

#include "../solver.hpp"

std::vector<clause_t> OneHot0(const std::vector<int> &vars) {
    std::vector<clause_t> clauses;
    for (size_t i = 0; i < vars.size() - 1; ++i)
        for (size_t j = i + 1; j < vars.size(); ++j)
            clauses.push_back({-vars[i], -vars[j]});
    return clauses;
}

std::vector<clause_t> OneHot(const std::vector<int> &vars) {
    std::vector<clause_t> clauses = OneHot0(vars);
    clauses.push_back(vars);
    return clauses;
}

static inline void display(const std::vector<int> assignments, int N) {
    for (int row = 0; row < N; ++row) {
        for (int col = 0; col < N; ++col) {
            std::cout << (assignments[row * N + col] > 0 ? "Q" : ".");
        }
        std::cout << "\n";
    }
}

int main(int argc, char **argv) {

    int N;
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " N\n";
        return 0;
    }
    N = std::atoi(argv[1]);

    std::vector<clause_t> R, C, DLR, DRL;

    // Exactly one queen must be placed on each row
    for (int row = 0; row < N; ++row) {
        std::vector<int> vars = {};
        for (int col = 1; col <= N; ++col)
            vars.push_back(row * N + col);
        auto clauses = OneHot(vars);
        std::copy(clauses.begin(), clauses.end(), std::back_inserter(R));
    }

    // Exactly one queen must be placed on each column
    for (int col = 1; col <= N; ++col) {
        std::vector<int> vars = {};
        for (int row = 0; row < N; ++row)
            vars.push_back(row * N + col);
        auto clauses = OneHot(vars);
        std::copy(clauses.begin(), clauses.end(), std::back_inserter(C));
    }

    // Diagonal Constraints
    // left-to-right
    std::vector<std::pair<int, int> > starts;
    std::vector<std::vector<std::pair<int, int> > > lrdiags;
    for (int row = N - 1; row > 1; --row)
        starts.emplace_back(row, 1);
    for (int col = 1; col < N; ++col)
        starts.emplace_back(1, col);
    for (const auto &pos : starts) {
        lrdiags.push_back({});
        int row = pos.first;
        int col = pos.second;
        while (row <= N && col <= N) {
            lrdiags.back().emplace_back(row, col);
            row++;
            col++;
        }
    }
    for (const auto &diag : lrdiags) {
        std::vector<int> vars = {};
        for (const auto &pos : diag) {
            vars.push_back((pos.first - 1) * N + pos.second);
        }
        auto clauses = OneHot0(vars);
        std::copy(clauses.begin(), clauses.end(), std::back_inserter(DLR));
    }

    // right-to-left
    starts.clear();
    std::vector<std::vector<std::pair<int, int> > > rldiags;
    for (int row = N - 1; row > 1; --row)
        starts.emplace_back(row, N);
    for (int col = N; col > 1; --col)
        starts.emplace_back(1, col);
    for (const auto &pos : starts) {
        rldiags.push_back({});
        int row = pos.first;
        int col = pos.second;
        while (row <= N && col >= 1) {
            rldiags.back().emplace_back(row, col);
            row++;
            col--;
        }
    }
    for (const auto &diag : rldiags) {
        std::vector<int> vars = {};
        for (const auto &pos : diag) {
            vars.push_back((pos.first - 1) * N + pos.second);
        }
        auto clauses = OneHot0(vars);
        std::copy(clauses.begin(), clauses.end(), std::back_inserter(DRL));
    }

    std::vector<clause_t> clauses;
    clauses.reserve(R.size() + C.size() + DLR.size() + DRL.size());
    std::copy(R.begin(), R.end(), std::back_inserter(clauses));
    std::copy(C.begin(), C.end(), std::back_inserter(clauses));
    std::copy(DLR.begin(), DLR.end(), std::back_inserter(clauses));
    std::copy(DRL.begin(), DRL.end(), std::back_inserter(clauses));

    int maxVarIndex = N * N;
    int nSolution = 0;

    while (true) {

        Solver solver(clauses, maxVarIndex);

        if (solver.DPLL()) {
            nSolution++;
            auto assignments = solver.getAssignments();
            std::cout << "\nSolution " << nSolution << "\n";
            display(assignments, N);
            if (N == 1) {
                std::cout << "\nNumber of solution to the " << N << " queens puzzle: " 
                        << nSolution << "\n";
                break;
            }
            for (auto &var : assignments)
                var *= -1;
            clauses.push_back(assignments);
        }
        else {
            std::cout << "\nNumber of solution to the " << N << " queens puzzle: " 
                      << nSolution << std::endl;
            break;
        }

    }

    return 0;
}
