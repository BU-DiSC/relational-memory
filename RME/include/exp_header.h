#include "config.h"
#include "parse_config.h"

void set_q2_exp_default_config(struct _config_db *config_db, struct experiment_config *exp_config, struct _config_query *query_config);
void set_q1_exp_default_config(struct _config_db *config_db, struct experiment_config *exp_config, struct _config_query *query_config);
void set_q3_exp_default_config(struct _config_db *config_db, struct experiment_config *exp_config, struct _config_query *query_config);
void set_q4_exp_default_config(struct _config_db *config_db, struct experiment_config *exp_config, struct _config_query *query_config);

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








