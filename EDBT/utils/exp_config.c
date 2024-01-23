#include "exp_header.h"

void set_config(int query_type, struct _config_db *config_db, struct experiment_config *exp_config, struct _config_query *query_config) {
    // Common configurations for all queries
    exp_config->r_col = COLUMN_WIDTH;
    exp_config->num_samples = 1;

    config_db->row_count = 43690;
    config_db->num_columns = 16;
    config_db->row_size = 64;
    config_db->store_type = 'r';
    config_db->col_type = 'r';
    config_db->print = false;
    config_db->min = 0;
    config_db->max = 1000;
    config_db->mvcc_enabled = false;

    // Specific configurations based on query type
    if (query_type == '1') {
        query_config->offsets = "0,8";
        query_config->enabled_column_number = 2;
        query_config->k_value = 0;
    } else if (query_type == '2' || query_type == '3') {
        query_config->offsets = "0,8";
        query_config->enabled_column_number = 2;
        query_config->k_value = 136;
    } else if (query_type == '4') {
        query_config->offsets = "0,4,12";
        query_config->enabled_column_number = 3;
        query_config->k_value = 136;
    } else {
        // q5 will be added soon
    }
}