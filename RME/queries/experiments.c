#include "exp_header.h"
#include "parse_config.h"
#include <sys/wait.h>

FILE* open_file(const char *filename, const char *mode) {
    FILE *file = fopen(filename, mode);
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE); // or return NULL and handle it in the caller
    }
    return file;
}

void row_size_exp(struct _config_db *config_db, struct experiment_config *exp_config, struct _config_query* query_config,
            QueryFunction query_func, char* filename) {

    FILE* fp = open_file(filename, "w");
    FILE *results_file = open_file("data/results_file.csv", "w");

    query_config->results = results_file;
    query_config->output_file = fp;
    fprintf(fp, "bench, mem, temp, row_size, row_count, col_width, cycles, l1_references, l1_refills, l2_references, l2_refills, inst_retired\n");
    fprintf(results_file, "bench, row_size, RME(cold), RME(hot), ROW, COL\n");
    for (unsigned int row_size = 16; row_size <= 256; row_size *= 2) {
        for (unsigned int sample = 1; sample <= exp_config->num_samples; sample++) {
            // set up parameters for the experiment
            config_db->store_type = 'r';
            config_db->row_size = row_size;
            config_db->num_columns = row_size / exp_config->r_col;
            config_db->column_widths = malloc(config_db->num_columns * sizeof(unsigned int));
            config_db->column_types = malloc(config_db->num_columns * sizeof(char));
            for (int i = 0; i < config_db->num_columns; i++) {
                config_db->column_widths[i] = exp_config->r_col;
                config_db->column_types[i] = 'r';
            }
            populate_column_widths(config_db,query_config);
            parse_exp_offsets(query_config);
            // run experiment for row store
            run_query(config_db, query_config, query_func);
            // run the experiment for column store
            config_db->store_type = 'c';
            run_query(config_db, query_config, query_func);
            free(config_db->column_widths);
            free(config_db->column_types);
        }
    }
    fclose(fp);
    fclose(results_file);
}

void projectivity_exp(struct _config_db *config_db, struct experiment_config *exp_config, struct _config_query* query_config,
                      QueryFunction query_func, char* filename) {
                        
    FILE* fp = open_file(filename, "w");
    FILE *results_file = open_file("data/results_file.csv", "w");

    query_config->results = results_file;
    query_config->output_file = fp;
    fprintf(fp, "bench, mem, temp, row_size, enabled_cols, row_count, col_width, cycles, l1_references, l1_refills, l2_references, l2_refills, inst_retired\n");
    fprintf(results_file, "bench, num_columns, RME(cold), RME(hot), ROW, COL\n");

    for (unsigned int num_columns = 2; num_columns <= 11; num_columns++) {
        // Directly set the offset values in the query_config's col_offsets array
        for (unsigned int sample = 1; sample <= exp_config->num_samples; sample++) {
            for (unsigned int i = 0; i < num_columns; i++) {
                query_config->col_offsets[i] = i * 4;  // fixed offset increment of 4
            }

            query_config->enabled_column_number = num_columns;
            config_db->store_type = 'r';
            config_db->row_size = 64;
            config_db->num_columns = 16;
            config_db->column_widths = malloc(config_db->num_columns * sizeof(unsigned int));
            config_db->column_types = malloc(config_db->num_columns * sizeof(char));
            for (int i = 0; i < config_db->num_columns; i++) {
                config_db->column_widths[i] = exp_config->r_col;
                config_db->column_types[i] = 'r';
            }
            populate_column_widths(config_db,query_config);
            // Set up and run the experiment for each number of columns
            config_db->store_type = 'r';
            run_query(config_db, query_config, query_func);

            config_db->store_type = 'c';
            run_query(config_db, query_config, query_func);
            free(config_db->column_widths);
            free(config_db->column_types);
        }
    }

    fclose(fp);
    fclose(results_file);
}
