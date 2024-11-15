#ifndef PARSE_CONFIG_H
#define PARSE_CONFIG_H

#include "config.h"

typedef void (*QueryFunction)(struct _config_db, struct _config_query, unsigned char * db);
typedef void (*ConfigFunction)(struct _config_db*, struct experiment_config*, struct _config_query*);

QueryFunction get_query_function(int query_type);
ConfigFunction get_config_function(int query_type);

int parse_exp_offsets(struct _config_query* query_config);
char* get_filename_from_query_type(char query_type);
void parse_args(int argc, char **argv, struct _config_db *config_db, struct experiment_config *exp_config, struct _config_query *query_config);
void parse_config_file(struct _config_db *config_db, struct experiment_config *exp_config);

#endif