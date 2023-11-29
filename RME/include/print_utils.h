#define _GNU_SOURCE

void print_experiment_config(struct experiment_config *config, struct _config_db config_db, struct _config_query config_query);
void print_cycles_from_file(const char *filename);
void print_query2_params(const struct _config_query *params);
void print_config(const struct _config_db *config_db);
void print_dotted_line(int n);
void print_help(const char *program_name);
void print_db(struct _config_db config, unsigned char* db, unsigned int row_size);
void print_config_info(struct _config *config);