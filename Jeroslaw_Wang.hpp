#pragma once

#include <cmath>
#include <vector>
#include <unordered_map>

#include "variable_selection.hpp"

class Jeroslaw_Wang : public branching_heuristic {

public:

    Jeroslaw_Wang() = default;

    Jeroslaw_Wang(const std::vector<clause_t> &clauses, int maxVarIndex, 
                  const std::vector<int> *assignments) {
        this->initialize(clauses, maxVarIndex, assignments);
    }

    void initialize(const std::vector<clause_t> &clauses, int maxVarIndex, 
                    const std::vector<int> *assignments) {

        this->assignments = assignments;
        this->score_table.reserve(2 * maxVarIndex);
        for (int i = 1; i <= maxVarIndex; ++i) {
            this->score_table[i] = 0.0;
            this->score_table[-i] = 0.0;
        }
        for (const auto &clause : clauses) {
            this->update(clause);
        }
    }

    /**
     * @brief Branching Heuristics - Jeroslaw-Wang Score
     * @return The variable @c x which will be assigned to 1 
     *         (if it's bigger than 0) or 0 instead  
     */
    virtual int getNextDicisionVariable() const override {
        int next_var = 0;
        double max_score = 0.0;
        for (size_t i = 1; i < this->assignments->size(); ++i) {
            if (this->assignments->at(i) == UNASSIGNED) {
                double score;
                if ((score = this->score_table.at(i)) > max_score) {
                    max_score = score;
                    next_var = i;
                }
                if ((score = this->score_table.at(-i)) > max_score) {
                    max_score = score;
                    next_var = -i;
                }
            }
        }
        return next_var;
    }

    /// Update score table
    virtual void update(const clause_t &clause) override {
        double score = std::pow(2, static_cast<int>(-clause.size()));
        for (auto var : clause)
            this->score_table[var] += score;
    }

private:

    std::unordered_map<int, double> score_table;
    const std::vector<int> *assignments;

};