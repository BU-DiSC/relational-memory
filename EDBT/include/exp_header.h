#include "config.h"
#include "parse_config.h"
#include <stdint.h>

void set_config(int query_type, struct _config_db *config_db, struct experiment_config *exp_config, struct _config_query *query_config);

void generate_db(struct _config_db config);
int configure_relcache(struct _config_db config, struct _config_query *params);
void call_db_reset_relcache(int value);
int reset_relcache(unsigned int frame_offset);

void run_query(struct _config_db *config_db, struct _config_query* query_config, QueryFunction query_func);
void run_query2(struct _config_db config_db, struct _config_query params);
void run_query3(struct _config_db config_db, struct _config_query params);
void run_query1(struct _config_db config_db, struct _config_query params);
void run_query4(struct _config_db config_db, struct _config_query params);

void row_size_exp(struct _config_db *config_db, struct experiment_config *exp_config, struct _config_query* query_config,
            QueryFunction query_func, char* filename);
void projectivity_exp(struct _config_db *config_db, struct experiment_config *exp_config, struct _config_query* query_config,
                      QueryFunction query_func, char* filename);

FILE* open_file(const char *filename, const char *mode);



struct perf_counters {
        long unsigned l1_references; ///< L1-D accesses
        long unsigned l1_refills; ///< L1-D misses
        long unsigned l2_references; ///< L2 accesses
        long unsigned l2_refills; ///< L2 misses
        long unsigned inst_retired; ///< Instructions retired
};

struct perf_counters pmcs_diff(struct perf_counters* a, struct perf_counters* b);

int teardown_pmcs(void);
void pmcs_get_value(struct perf_counters* res);
int setup_pmcs(void);

#define magic_timing_begin(cycleLo, cycleHi) {\
  uint32_t low, high;\
  asm volatile("rdtsc" : "=a" (low), "=d" (high));\
  *cycleLo = low;\
  *cycleHi = high;\
}

#define magic_timing_end(cycleLo, cycleHi) {\
  uint32_t low, high;\
  asm volatile("rdtsc" : "=a" (low), "=d" (high));\
  *cycleLo = ((uint64_t)high << 32 | low) - ((uint64_t)(*cycleHi) << 32 | *cycleLo);\
  *cycleHi = 0;\
}
