#include "solver.hpp"

#include <iostream>
#include <algorithm>
#include <iterator>
#include <cmath>
#include <unordered_set>
#include <unordered_map>
#include <cassert>

#define MIN_LEN_OF_LEARNED_CLAUSE 10
#define CLAUSES_CAPACITY_MULTIPLIER 100

Solver::Solver(std::vector<clause_t> &clauses, int maxVarIndex) {

    this->clauses = clauses;
    this->maxVarIndex = maxVarIndex;
    this->nConflicts = this->nDecisions = this->nRestarts = 0U;
    this->nextRestart = this->luby.next();

    // To prevent reallocation of vector which makes pointer to clause invaild
    this->clauses_capacity = CLAUSES_CAPACITY_MULTIPLIER * this->clauses.size();
    this->clauses.reserve(this->clauses_capacity);
    this->assigned_levels_reverse.resize(maxVarIndex + 1, -1);
    this->assigned_levels.resize(maxVarIndex + 1);
    this->assignments.resize(maxVarIndex + 1, UNASSIGNED);
    this->pos_watched.resize(maxVarIndex + 1);
    this->neg_watched.resize(maxVarIndex + 1);
    this->selector = new VSIDS(clauses, maxVarIndex, &this->assignments, &this->nConflicts);

    // Construct Watching Lists
    for (auto &clause : this->clauses) {
        this->constructWatchingLists(clause);
    }
}

void Solver::assign(int var, const clause_t *clause, int level/*=0*/) {

#ifdef DEBUG
    std::clog << (this->assigned_levels[level].empty() ? "Decide " : "Imply ") 
              << var << " on level " << level << "\n";
#endif

    this->assignments[std::abs(var)] = (var > 0) ? TRUE : FALSE;
    this->assigned_levels_reverse[std::abs(var)] = level;
    this->assigned_levels[level].emplace_back(var, clause);
    this->imply_queue.push(var);
}

void Solver::unassign(int level) {

#ifdef DEBUG
    std::clog << "Unassign ";
    for (auto assigned : this->assigned_levels[level])
        std::clog << assigned.first << " ";
    std::clog << "at level " << level << "\n";
#endif

    std::unordered_set<int> assigned_vars_in_level_0;
    for (auto assigned : this->assigned_levels[0]) {
        assigned_vars_in_level_0.insert(assigned.first);
        assigned_vars_in_level_0.insert(-assigned.first);
    }

    for (auto assigned : this->assigned_levels[level]) {
        // Special case: When newly learned clause is an unit clause,
        // the only one variable of it is assigned at level 0, 
        // so the variable cannnot be unassigned  
        if (!assigned_vars_in_level_0.count(assigned.first)) {
            this->assignments[std::abs(assigned.first)] = UNASSIGNED;
            this->assigned_levels_reverse[std::abs(assigned.first)] = -1;
        }
    }
    this->assigned_levels[level].clear();
}

bool Solver::isWatched(const clause_t *clause, int x) const {
    const auto &watching_vars = this->watched_variable.at(clause);
    return x == watching_vars.first || x == watching_vars.second;
}

void Solver::replaceWatchingVariable(const clause_t *clause, int from, int to) {
    auto &watching_vars = this->watched_variable[clause];
    if (from == watching_vars.first)
        watching_vars.first = to;
    else
        watching_vars.second = to;
}

int Solver::BCP(int x, int level) {

    std::list<const clause_t *> &watching = (x < 0) ? this->pos_watched[-x] : this->neg_watched[x];
    std::list<const clause_t *>::iterator it = watching.begin();

    while (it != watching.end()) {

        const clause_t *clause = *it;
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
                std::list<const clause_t *> &y_watching = (y > 0) ? 
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
                this->assign(other_watched_var, clause, level);
            }
            // Case 4
            else if ((assignment == FALSE && other_watched_var > 0) ||
                     (assignment == TRUE  && other_watched_var < 0)) {

                // Run 1UIP to get newly learned clause and decide jump level
                clause_t learned_clause = this->FirstUIP(clause, level);
                if (learned_clause.size() > MIN_LEN_OF_LEARNED_CLAUSE || 
                    this->clauses.size() >= this->clauses_capacity)
                    return ECONFLICT;

                if (++this->nConflicts == this->nextRestart) {
                    this->nRestarts++;
                    this->nextRestart += this->luby.next();
                    this->imply_queue = {};
#ifdef DEBUG
                    std::clog << "Restart #" << this->nRestarts << "\n";
#endif
                    this->jump_to = 0;
                    return ECONFLICT;
                }

                this->jump_to = INT32_MIN;
                for (auto var : learned_clause) {
                    int l = this->assigned_levels_reverse[std::abs(var)];
                    if (l != level)
                        this->jump_to = std::max(this->jump_to.value(), l);
                }
                this->jump_to = std::max(this->jump_to.value(), 0);

                // Add it to database
                assert("Learned clause should not be empty" && !learned_clause.empty());
                this->clauses.push_back(learned_clause);

                // Update score table
                this->selector->update(learned_clause);

                // Update some variables associated with 2-literals watching
                this->imply_queue = {};
                constructWatchingLists(this->clauses.back());
                if (learned_clause.size() > 1) {
                    int j = 0;
                    for (size_t i = 0; i < learned_clause.size() && j < 2; ++i) {
                        if (this->assigned_levels_reverse[std::abs(learned_clause[i])] != level) {
                            this->imply_queue.push(-learned_clause[i]);
                            j++;
                        }
                    }
                }
#ifdef DEBUG
                std::clog << "Learned clause: ";
                std::copy(learned_clause.begin(), learned_clause.end(), 
                          std::ostream_iterator<int>(std::clog, " "));
                std::clog << "\nJump to level " << this->jump_to.value() << "\n";
#endif
                return ECONFLICT;
            }
            // Case 3
            else {}
            ++it;
        }
    }
    return SUCCESS;
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

    while (true) {

        while (!this->imply_queue.empty()) {
            int var = this->imply_queue.front();
            this->imply_queue.pop();
            if(this->BCP(var, level) == ECONFLICT)
                return UNSAT;
        }

        int next_var = this->selector->getNextDicisionVariable();
        if (next_var == 0)
            return SAT;
        this->nDecisions++;        

        this->assign(next_var, nullptr, level + 1);
        if (DPLL(level + 1) == SAT)
            return SAT;

        this->unassign(level + 1);

        // First time conflict happened but learned no clause
        if (!this->jump_to.has_value()) {
            this->imply_queue = {};
            this->assign(-next_var, nullptr, level + 1);
            if (DPLL(level + 1) == SAT)
                return SAT;
            this->unassign(level + 1);
        }

        if (this->jump_to.has_value()) {
            if (this->jump_to.value() != level)
                return UNSAT;
            else {
                // Do BCP again on newly added unit clause
                this->jump_to = std::nullopt;
                continue;
            }
        }
        // Second time conflict happened but learned no clause
        else {
            this->imply_queue = {};
            return UNSAT;
        }
    }
}

std::vector<int> Solver::getAssignments() const {
    std::vector<int> assignments;
    assignments.resize(this->maxVarIndex);
    for (size_t i = 1; i < this->assignments.size(); ++i)
        assignments[i - 1] = (this->assignments[i] == TRUE) ? i : -i;
    return assignments;
}

clause_t Solver::resolve(const clause_t *F, const clause_t *G, int x) {

    clause_t clause;
    std::unordered_set<int> resolvent;
    clause.reserve(F->size() + G->size());
    resolvent.reserve(F->size() + G->size());

    std::copy(F->begin(), F->end(), std::inserter(resolvent, resolvent.end()));
    std::copy(G->begin(), G->end(), std::inserter(resolvent, resolvent.end()));
    resolvent.erase(x);
    resolvent.erase(-x);
    std::copy(resolvent.begin(), resolvent.end(), std::inserter(clause, clause.end()));

    return clause;
}

clause_t Solver::FirstUIP(const clause_t *conflicting_clause, int level) const {

    clause_t C = *conflicting_clause;
    int current_decision_var = this->assigned_levels.at(level).at(0).first;

    while (true) {

        int nAssignedAtCurrentLevel = 0;
        const clause_t *antecedent = nullptr;
        int p = 0;

        nAssignedAtCurrentLevel = std::count_if(C.begin(), C.end(), 
                                                [this, level](int var) {
                                                    return this->assigned_levels_reverse[std::abs(var)] == level;
                                                });

        if (nAssignedAtCurrentLevel <= 1)
            break;

        // Select most recently assigned variable in current decision level
        auto reverse_it = std::find_first_of(this->assigned_levels.at(level).rbegin(),
                                             this->assigned_levels.at(level).rend(),
                                             C.begin(), C.end(), 
                                             [=](std::pair<int, const clause_t *> a, int b) {
                                                 return std::abs(a.first) == std::abs(b) && 
                                                        std::abs(b) != std::abs(current_decision_var);
                                             });

        if (reverse_it != this->assigned_levels.at(level).rend()) {
            p = reverse_it->first;
            antecedent = reverse_it->second;
        }

        assert("antecedent cannot be nullptr" && antecedent != nullptr);
        assert("p should not be 0" && p != 0);

        C = this->resolve(&C, antecedent, p);
    }
    return C;
}

void Solver::constructWatchingLists(const clause_t &clause) {

    int var1 = clause[0];
    if (var1 > 0)
        this->pos_watched[var1].push_back(&clause);
    else
        this->neg_watched[-var1].push_back(&clause);
    
    if (clause.size() < 2) {
        this->watched_variable[&clause] = {var1, 0};
        this->assign(var1, &clause);
        return;
    }

    int var2 = clause[1];
    if (var2 > 0)
        this->pos_watched[var2].push_back(&clause);
    else
        this->neg_watched[-var2].push_back(&clause);
    this->watched_variable[&clause] = {var1, var2};
}

void Solver::printStatistics() const {
    std::clog << "\nrestarts              : " << this->nRestarts
              << "\nconflicts             : " << this->nConflicts
              << "\ndecisions             : " << this->nDecisions
              << "\n";
}