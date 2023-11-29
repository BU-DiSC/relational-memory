#include "exp_header.h"

void set_q1_exp_default_config(struct _config_db *config_db, struct experiment_config *exp_config, struct _config_query *query_config) {
    exp_config->r_col = 4; //fixed column width for the varying row size experiment
    query_config->offsets = "0,8";
    exp_config->num_samples = 10;
    query_config->enabled_column_number = 2;
    query_config->k_value = 0;
    // set database configuration for the experiment
    config_db->row_count = 43690;
    config_db->store_type = 'r';
    config_db->print = false;
    config_db->min = 0;
    config_db->max = 1000;
    config_db->mvcc_enabled = false;
}

void set_q2_exp_default_config(struct _config_db *config_db, struct experiment_config *exp_config, struct _config_query *query_config) {
    // set experiment configuration
    exp_config->r_col = 4; //fixed column width for the varying row size experiment
    exp_config->num_samples = 10;

    // set query configuration
    query_config->offsets = "0,8";
    query_config->k_value = 136;
    query_config->enabled_column_number = 2;

    // set database configuration for the experiment
    config_db->row_count = 43690;
    config_db->store_type = 'r';
    config_db->print = false;
    config_db->min = 0;
    config_db->max = 1000;
    config_db->mvcc_enabled = false;
}

void set_q3_exp_default_config(struct _config_db *config_db, struct experiment_config *exp_config, struct _config_query *query_config) {
    exp_config->r_col = 4; //fixed column width for the varying row size experiment
    query_config->offsets = "0,8";
    exp_config->num_samples = 10;
    query_config->enabled_column_number = 2;
    query_config->k_value = 136;
    // set database configuration for the experiment
    config_db->row_count = 43690;
    config_db->store_type = 'r';
    config_db->print = false;
    config_db->min = 0;
    config_db->max = 1000;
    config_db->mvcc_enabled = false;
}

void set_q4_exp_default_config(struct _config_db *config_db, struct experiment_config *exp_config, struct _config_query *query_config) {
    query_config->offsets = "0,4,12";
    exp_config->r_col = 4; //fixed column width for the varying row size experiment
    exp_config->num_samples = 10;
    query_config->enabled_column_number = 3;
    query_config->k_value = 136;
    // set database configuration for the experiment
    config_db->row_count = 43690;
    config_db->store_type = 'r';
    config_db->print = false;
    config_db->min = 0;
    config_db->max = 1000;
    config_db->mvcc_enabled = false;
}