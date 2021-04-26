#pragma once

#include <map>
#include <unordered_map>
#include <vector>
#include <list>
#include <queue>
#include <utility>
#include <optional>

typedef std::vector<int> clause_t;

class Solver {

    enum {
        ECONFLICT, SUCCESS
    };

    enum {
        UNASSIGNED, TRUE, FALSE
    };

    enum {
        UNSAT, SAT, UNSOLVED
    };

private:

    int maxVarIndex;
    size_t clauses_capacity;
    std::vector<clause_t> clauses;
    /// Final answer
    std::vector<int> assignments;
    /// Key is level and value is a vector storing pairs of a variable 
    /// and a pointer to clause which makes it unit
    std::map<int, std::vector<std::pair<int, const clause_t *> > > assigned_levels;
    /// The value of i-th element is the decision (or implied) level of variable i
    std::vector<int> assigned_levels_reverse;
    /// 2-Literal Watching
    std::vector<std::list<const clause_t *> > pos_watched;
    std::vector<std::list<const clause_t *> > neg_watched;
    /// Store the 2 watched literals on clauses
    std::unordered_map<const clause_t *, std::pair<int, int> > watched_variable;
    /// Store x (or -x) if x is to be implied as 1 (or 0)
    std::queue<int> imply_queue;
    /// Branching Heuristics - Jeroslaw-Wang Score table
    std::unordered_map<int, double> score_table;
    /// Denote which level to jump to rerun BCP if conflicting
    std::optional<int> jump_to;

public:

    Solver(std::vector<clause_t> &clauses, int maxVarIndex);

    void assign(int var, const clause_t *clause, int level=0);

    void unassign(int level);

    /**
     * @return true if @c x in @c clause is been watching
     */
    bool isWatched(const clause_t *clause, int x) const;

    void replaceWatchingVariable(const clause_t *clause, int from, int to);

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
     * @brief Implementation of modified Davis-Putnam-Logemann-Loveland algorithm
     *        with non-chronological backtracking
     * @retval SAT if SAT
     * @retval UNSAT if UNSAT 
     */
    bool DPLL(int level=0);

    /**
     * @brief Convert the final assignments to DIMACS format
     */
    std::vector<int> getAssignments() const;

    /**
     * @brief Resolve clauses @c F and @c G on @c x
     * @return The resolvent
     */
    static clause_t resolve(const clause_t *F, const clause_t *G, int x);

    /**
     * @brief Implementation of 1UIP algorithm
     * @return The conflicting clause associated with the 1UIP cut
     */
    clause_t FirstUIP(const clause_t *conflicting_clause, int level) const;

    void constructWatchingLists(const clause_t &clause);
};