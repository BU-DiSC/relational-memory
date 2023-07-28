/** @file performance_counters.h
 * @ingroup base
 * @author Denis Hoornaert
 * @brief API to extract performance counters (e.g., L1-D refills) for the benchmark under analysis.
 */
#ifndef PERFORMANCE_COUNTERS_H
#define PERFORMANCE_COUNTERS_H


/** @brief Struct used to hold the measured performance events.
 * @details
 * Struct returning the performance counters in an abstract way.
 */
struct perf_counters {
        long unsigned l1_references; ///< L1-D accesses
        long unsigned l1_refills; ///< L1-D misses
        long unsigned l2_references; ///< L2 accesses
        long unsigned l2_refills; ///< L2 misses
        long unsigned inst_retired; ///< Instructions retired
};

/** @brief Enable user-space access to performance counters.
 * @return Group_fd head's pid on sucess, -1 on error.
 */
int setup_pmcs(void);

/** @brief Close access to performance counters.
 * @return 0 on sucess, -1 on error.
 */
int teardown_pmcs(void);

/** @brief Read performance counters value.
 * @return struct perf_countrers.
 */
void pmcs_get_value(struct perf_counters* res);

struct perf_counters pmcs_diff(struct perf_counters* a, struct perf_counters* b);

#endif /* PERFORMANCE_COUNTERS_H */
