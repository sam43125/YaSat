#pragma once

#include <vector>

typedef std::vector<int> clause_t;

class branching_heuristic {

protected:

    enum {
        UNASSIGNED, TRUE, FALSE
    };

    virtual int getNextDicisionVariable() const = 0;
    virtual void update(const clause_t &clause) = 0;

};