#include "solver.hpp"

#include <iostream>
#include <algorithm>
#include <iterator>

Solver::Solver(std::vector<clause_t> &clauses, int maxVarIndex) {

    this->clauses = clauses;
    this->maxVarIndex = maxVarIndex;

    this->assignments.resize(maxVarIndex + 1, UNASSIGNED);
    this->pos_watched.resize(maxVarIndex + 1);
    this->neg_watched.resize(maxVarIndex + 1);

    // Construct Watching Lists
    for (auto &clause : this->clauses) {

        int var1 = clause[0];
        if (var1 > 0)
            this->pos_watched[var1].push_back(&clause);
        else
            this->neg_watched[-var1].push_back(&clause);
        
        if (clause.size() < 2) {
            this->watched_variable[&clause] = {var1, 0};
            this->assign(var1);
            continue;
        }

        int var2 = clause[1];
        if (var2 > 0)
            this->pos_watched[var2].push_back(&clause);
        else
            this->neg_watched[-var2].push_back(&clause);
        this->watched_variable[&clause] = {var1, var2};
    }
}

void Solver::assign(int var, int level/*=0*/) {
#ifdef DEBUG
    std::clog << (this->assigned_levels[level].empty() ? "Decide " : "Imply ") 
              << var << " on level " << level << "\n";
#endif
    this->assignments[std::abs(var)] = (var > 0) ? TRUE : FALSE;
    this->assigned_levels[level].push_back(var);
    this->imply_queue.push(var);
}

void Solver::unassign(int level) {
#ifdef DEBUG
    std::clog << "Unassign ";
    std::copy(this->assigned_levels[level].begin(), this->assigned_levels[level].end(), 
              std::ostream_iterator<int>(std::clog, " "));
    std::clog << "at level " << level << "\n";
#endif
    for (auto var : this->assigned_levels[level]) {
        this->assignments[std::abs(var)] = UNASSIGNED;
    }
    this->imply_queue = {};
    this->assigned_levels[level].clear();
}

bool Solver::isWatched(clause_t *clause, int x) {
    auto watching_vars = this->watched_variable[clause];
    return x == watching_vars.first || x == watching_vars.second;
}

void Solver::replaceWatchingVariable(clause_t *clause, int from, int to) {
    auto &watching_vars = this->watched_variable[clause];
    if (from == watching_vars.first)
        watching_vars.first = to;
    else
        watching_vars.second = to;
}

int Solver::BCP(int x, int level) {

    std::list<clause_t *> &watching = (x < 0) ? this->pos_watched[-x] : this->neg_watched[x];
    std::list<clause_t *>::iterator it = watching.begin();

    while (it != watching.end()) {

        clause_t *clause = *it;
        bool case1 = false;

        for (auto y : *clause) {
            if ((this->assignments[std::abs(y)] == FALSE && y > 0) ||
                (this->assignments[std::abs(y)] == TRUE  && y < 0))
                continue;
            // Case 0 and Case 1
            if ((((this->assignments[std::abs(y)] == FALSE && y < 0) ||
                  (this->assignments[std::abs(y)] == TRUE  && y > 0)) || 
                  (this->assignments[std::abs(y)] == UNASSIGNED)) && (!this->isWatched(clause, y))) {
                it = watching.erase(it);
                std::list<clause_t *> &y_watching = (y > 0) ? 
                                                    this->pos_watched[y] : 
                                                    this->neg_watched[-y];
                y_watching.push_back(clause);
                this->replaceWatchingVariable(clause, -x, y);
                case1 = true;
                break;
            }
        }

        if (!case1) {

            auto watched_vars = this->watched_variable[clause];
            int other_watched_var = (-x == watched_vars.first) ? 
                                    watched_vars.second : 
                                    watched_vars.first;
            int &assignment = this->assignments[std::abs(other_watched_var)];

            // Case 2
            if (assignment == UNASSIGNED) {
                this->assign(other_watched_var, level);
            }
            // Case 4
            else if ((assignment == FALSE && other_watched_var > 0) ||
                        (assignment == TRUE  && other_watched_var < 0)) {
                return ECONFLICT;
            }
            // Case 3
            else {}
            ++it;
        }
    }
    return SUCCESS;
}

int Solver::getNextDicisionVariable() const {
    // Naive way
    for (size_t i = 1; i < this->assignments.size(); ++i)
        if (this->assignments[i] == UNASSIGNED)
            return i;
    return 0;
}

int Solver::isSolved() const {

    std::vector<int> status;
    status.resize(this->clauses.size());

    for (size_t i = 0; i < this->clauses.size(); ++i) {
        const std::vector<int> &clause = this->clauses[i];
        status[i] = UNSAT;
        for (auto var : clause) {
            if (this->assignments[std::abs(var)] == UNASSIGNED) {
                status[i] = UNSOLVED;
            }
            else if ((this->assignments[std::abs(var)] == TRUE  && var > 0) ||
                     (this->assignments[std::abs(var)] == FALSE && var < 0)) {
                status[i] = SAT;
                break;
            }
        }
    }

    if (std::all_of(status.begin(), status.end(), [](int s) {return s == SAT;}))
        return SAT;
    else if (std::any_of(status.begin(), status.end(), [](int s) {return s == UNSAT;}))
        return UNSAT;
    else
        return UNSOLVED;
}

bool Solver::DPLL(int level/*=0*/) {

    while (!this->imply_queue.empty()) {
        int var = this->imply_queue.front();
        this->imply_queue.pop();
        if(this->BCP(var, level) == ECONFLICT) 
            return UNSAT;
    }

    int status = this->isSolved();
    if (status == SAT)
        return SAT;
    else if (status == UNSAT)
        return UNSAT;

    ++level;
    
    int next_var = this->getNextDicisionVariable();
    if (next_var == 0)
        return SAT;
    
    this->assign(next_var, level);
    if (DPLL(level) == SAT)
        return SAT;
    
    this->unassign(level);
    this->assign(-next_var, level);
    if (DPLL(level) == SAT)
        return SAT;
    else {
        this->unassign(level);
        return UNSAT;
    }
}

std::vector<int> Solver::getAssignments() const {
    std::vector<int> assignments;
    assignments.resize(this->maxVarIndex);
    for (size_t i = 1; i < this->assignments.size(); ++i)
        assignments[i - 1] = (this->assignments[i] == TRUE) ? i : -i;
    return assignments;
}