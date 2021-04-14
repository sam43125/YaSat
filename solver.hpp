#pragma once

#include <map>
#include <unordered_map>
#include <vector>
#include <list>
#include <queue>

typedef std::vector<int> clause_t;

enum {
    ECONFLICT, SUCCESS
};

enum {
    UNASSIGNED, TRUE, FALSE
};

enum {
    UNSAT, SAT, UNSOLVED
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
    /// Branching Heuristics - Jeroslaw-Wang Score table
    std::unordered_map<size_t, double> score_table;

public:

    Solver(std::vector<clause_t> &clauses, int maxVarIndex);

    void assign(int var, int level=0);

    void unassign(int level);

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
     * @brief Branching Heuristics - Jeroslaw-Wang Score
     * @return The variable @c x which will be assigned to 1 
     *         (if it's bigger than 0) or 0 instead  
     */
    int getNextDicisionVariable() const;

    /**
     * @retval SAT if all the clauses are solved
     * @retval UNSAT if one of the clauses is UNSAT
     * @retval UNSOLVED else
     */
    int isSolved() const;

    /**
     * @brief Implementation of Davis-Putnam-Logemann-Loveland algorithm
     * @retval SAT if SAT
     * @retval UNSAT if UNSAT 
     */
    bool DPLL(int level=0);

    /**
     * @brief Convert the final assignments to DIMACS format
     */
    std::vector<int> getAssignments() const;
};