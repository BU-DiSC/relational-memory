#include "config.h"
#include "parse_config.h"
#include <stdint.h>

void set_config(int query_type, struct _config_db *config_db, struct experiment_config *exp_config, struct _config_query *query_config);

unsigned char* generate_db(struct _config_db config);
int configure_relcache(struct _config_db config, struct _config_query *params);
void call_db_reset_relcache(int value);
int reset_relcache(unsigned int frame_offset);

void run_query(struct _config_db *config_db, struct _config_query* query_config, QueryFunction query_func);
void run_query1(struct _config_db config_db, struct _config_query params, unsigned char * db);
void run_query2(struct _config_db config_db, struct _config_query params, unsigned char * db);
void run_query3(struct _config_db config_db, struct _config_query params, unsigned char * db);
void run_query4(struct _config_db config_db, struct _config_query params, unsigned char * db);

void row_size_exp(struct _config_db *config_db, struct experiment_config *exp_config, struct _config_query* query_config,
            QueryFunction query_func, char* filename);
void col_size_exp(struct _config_db *config_db, struct experiment_config *exp_config, struct _config_query* query_config,
            QueryFunction query_func, char* filename);
void projectivity_exp(struct _config_db *config_db, struct experiment_config *exp_config, struct _config_query* query_config,
                      QueryFunction query_func, char* filename);

FILE* open_file(const char *filename, const char *mode);


#ifdef __aarch64__
#define magic_timing_begin(cycleLo, cycleHi){\
  *cycleHi=0;\
  asm volatile("mrs %0, CNTVCT_EL0": "=r"(*cycleLo) );\
}
#define magic_timing_end(cycleLo, cycleHi){\
  unsigned tempCycleLo, tempCycleHi =0;\
  asm volatile("mrs %0, CNTVCT_EL0":"=r"(tempCycleLo) );\
  *cycleLo = tempCycleLo - *cycleLo;\
  *cycleHi = tempCycleHi - *cycleHi;\
}
#else
#define magic_timing_begin(cycleLo, cycleHi){\
  *cycleLo=0;\
  *cycleHi=0;\
}
#define magic_timing_end(cycleLo, cycleHi){\
  unsigned tempCycleLo, tempCycleHi =0;\
  *cycleLo = tempCycleLo - *cycleLo;\
  *cycleHi = tempCycleHi - *cycleHi;\
}
#endif