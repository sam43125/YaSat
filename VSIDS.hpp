#pragma once

#include <map>
#include <list>
#include <vector>
#include <utility>
#include <iostream>

#include "variable_selection.hpp"

#define decay_freq 128U

class VSIDS : public branching_heuristic {

public:

    VSIDS() = default;

    VSIDS(const std::vector<clause_t> &clauses, int maxVarIndex, 
          const std::vector<int> *assignments, const unsigned *nConflicts) {

        this->nConflicts = nConflicts;
        this->maxVarIndex = maxVarIndex;
        this->assignments = assignments;
        this->scores_reverse.resize(maxVarIndex + 1, {0, 0});

        for (const auto &clause : clauses)
            for (int var : clause) {
                if (var > 0)
                    this->scores_reverse[var].first++;
                else
                    this->scores_reverse[-var].second++;
            }

        for (int var = 1; var <= maxVarIndex; ++var) {
            this->scores[this->scores_reverse[var].first].push_back(var);
            this->scores[this->scores_reverse[var].second].push_back(-var);
        }
    }

    /**
     * @brief Branching Heuristics - Variable State Independent Decaying Sum
     * @return The variable @c x which will be assigned to 1 
     *         (if it's bigger than 0) or 0 instead  
     */
    virtual int getNextDicisionVariable() const override {
        auto start = this->scores.crbegin();
        auto end = this->scores.crend();
        for (auto it = start; it != end; ++it) {
            const auto &vars = it->second;
            for (auto var = vars.crbegin(); var != vars.crend(); ++var)
                if (this->assignments->at(std::abs(*var)) == UNASSIGNED)
                    return *var;
        }
        return 0;
    }

    virtual void update(const clause_t &clause) override {
        for (int var : clause) {
            int old_score;
            if (var > 0)
                old_score = this->scores_reverse[var].first++;
            else
                old_score = this->scores_reverse[-var].second++;
            this->scores[old_score].remove(var);
            this->scores[old_score + 1].push_back(var);
        }
        if (*this->nConflicts % decay_freq == 0U)
            this->decay();
    }

    void decay() {

#ifdef DEBUG
        std::clog << "Decay scores in " <<  __PRETTY_FUNCTION__ << "\n";
#endif
        this->scores.clear();
        for (int var = 1; var <= maxVarIndex; ++var) {
            this->scores[(this->scores_reverse[var].first /= 2)].push_back(var);
            this->scores[(this->scores_reverse[var].second /= 2)].push_back(-var);
        }
    }

private:

    /// Key are scores and items are lists of variables 
    std::map<int, std::list<int> > scores;
    /// i-th element is pair of positive and negative scores
    std::vector<std::pair<int, int> > scores_reverse;
    int maxVarIndex;
    const std::vector<int> *assignments;
    const unsigned *nConflicts;

};