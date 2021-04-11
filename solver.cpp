#include "solver.hpp"

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
            this->imply_queue.push(var1);
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
    this->assignments[std::abs(var)] = (var > 0) ? TRUE : FALSE;
    this->assigned_levels[level].push_back(var);
}

bool Solver::isWatched(clause_t *clause, int x) {
    auto watching_vars = this->watched_variable[clause];
    return x == watching_vars.first || x == watching_vars.second;
}

void Solver::replaceWatchingVariable(clause_t *clause, int from, int to) {
    auto watching_vars = this->watched_variable[clause];
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
            if ((this->assignments[y] == FALSE && y > 0) ||
                (this->assignments[y] == TRUE  && y < 0))
                continue;
            // Case 1
            if (this->assignments[y] == UNASSIGNED && (!this->isWatched(clause, y))) {
                it = watching.erase(it);
                std::list<clause_t *> &y_watching = (y > 0) ? 
                                                    this->pos_watched[y] : 
                                                    this->neg_watched[-y];
                y_watching.push_back(clause);
                this->replaceWatchingVariable(clause, x, y);
                case1 = true;
                break;
            }
        }

        if (!case1) {

            auto watched_vars = this->watched_variable[clause];
            int other_watched_var = (x == watched_vars.first) ? 
                                    watched_vars.second : 
                                    watched_vars.first;
            int &assignment = this->assignments[std::abs(other_watched_var)];

            // Case 2
            if (assignment == UNASSIGNED) {
                this->assign(other_watched_var, level);
                // assignment = (other_watched_var < 0) ? FALSE : TRUE;
                this->imply_queue.push(other_watched_var);
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

bool Solver::solve(int level/*=0*/) {
    return false;
}