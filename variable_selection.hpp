#pragma once

#include <vector>

typedef std::vector<int> clause_t;

class branching_heuristic {

public:

    enum {
        UNASSIGNED, TRUE, FALSE
    };

    virtual ~branching_heuristic() {};

    virtual int getNextDicisionVariable() const = 0;
    virtual void update(const clause_t &clause) = 0;

};