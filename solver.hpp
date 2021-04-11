#pragma once

#include <map>
#include <unordered_map>
#include <vector>
#include <list>
#include <queue>

typedef std::vector<int>::iterator vIter;
typedef std::vector<int> clause_t;

enum {
    ECONFLICT, SUCCESS
};

enum {
    UNASSIGNED, TRUE, FALSE
};

class Solver {

private:

    int maxVarIndex;
    std::vector<clause_t> clauses;
    /// Final answer
    std::vector<int> assignments;
    std::map<int, std::vector<int> > assigned_levels;
    /// 2-Literal Watching
    std::vector<std::list<clause_t *> > pos_watched;
    std::vector<std::list<clause_t *> > neg_watched;
    /// Store the 2 watched literals on clauses
    std::unordered_map<clause_t *, std::pair<int, int> > watched_variable;
    /// Store x (or -x) if x is to be implied as 1 (or 0)
    std::queue<int> imply_queue;

public:

    Solver(std::vector<clause_t> &clauses, int maxVarIndex);

    void assign(int var, int level=0);

    /**
     * @return true if @c x in @c clause is been watching
     */
    bool isWatched(clause_t *clause, int x);

    void replaceWatchingVariable(clause_t *clause, int from, int to);

    /**
     * @param[in] x The decision (or implied) variable on previous step,
     *            if it's positive (or negative), it was assigned to 1 (or 0)
     * @param[in] level Decision (or implication) level
     * @retval ECONFLICT if conflict
     * @retval SUCCESS if no error
     */
    int BCP(int x, int level);

    /**
     * @retval true if SAT
     * @retval false if UNSAT 
     */
    bool solve(int level=0);

};