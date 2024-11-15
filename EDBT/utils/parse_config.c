#include "parse_config.h"
#include "exp_header.h"
#include <sys/stat.h>

int parse_exp_offsets(struct _config_query* query_config) {
    char *temp = strdup(query_config->offsets);  // Duplicate the string because strtok modifies the input
    char *token = strtok(temp, ",");
    int index = 0;
    while (token != NULL && index < MAX_GROUPS) {
        query_config->col_offsets[index] = (unsigned short) atoi(token);
        token = strtok(NULL, ",");
        index++;
    }
    free(temp);
    return index;
}

QueryFunction get_query_function(int query_type) {
    switch(query_type) {
        case '1':
            return run_query1;
        case '2':
            return run_query2;
        case '3':
            return run_query3;
        case '4':
            return run_query4;
        default:
            return NULL;
    }
}

char* get_filename_from_query_type(char query_type) {
    struct stat st = {0};

    static char filename[50]; // Static because we're returning the local variable's address
    snprintf(filename, sizeof(filename), "PLT2_result_q%c_col.csv", query_type);
    return filename;
}

void parse_args(int argc, char **argv, struct _config_db *config_db, struct experiment_config *exp_config, struct _config_query *query_config) {
    int opt;
    while ((opt = getopt(argc, argv, "C:R:S:T:m:M:V:O:K:N:PL")) != -1) {
        switch (opt) {
            case 'C': {                
                config_db->num_columns = atoi(optarg);
                config_db->row_size = config_db->num_columns * COLUMN_WIDTH;
                break;
            }
            case 'R': {
                config_db->row_count = atoi(optarg);
                break;
            }
            case 'T': {
                char c_type = *optarg;
                if (c_type != 'r' && c_type != 's' && c_type != 'z') {
                    fprintf(stderr, "Usage: [-T] array of column types (s: sorted, r:random, z:zero padded)\n");
                    exit(EXIT_FAILURE);
                }
                config_db->col_type = c_type;
                break;
            }
            case 'V': 
                config_db->mvcc_enabled = true; 
                break;
            case 'S': {
                char type = *optarg;
                if ( type != 'r' && type != 'c' ){
                    fprintf(stderr, "Usage: %s [-S] r: row store, c: column store (default: r)\n", argv[0]);
                    exit(EXIT_FAILURE);
                }
                //printf("%s", optarg);
                config_db->store_type = type; break;
            }
            case 'P': 
                config_db->print = true; 
                break;
            case 'm': 
                config_db->min = atoi(optarg); 
                break;
            case 'M': 
                config_db->max = atoi(optarg); 
                break;
            case 'N': {                
                exp_config->num_samples = atoi(optarg);
                break;
            }
            case 'O': { // Assuming 'O' is for col_offsets
                query_config->offsets = strdup(optarg);
                break;
            }
            case 'K': { // Assuming 'K' is for k_value
                query_config->k_value = atoi(optarg);
                break;
            }
            case 'L':{
                config_db->load_file = true;
            }
        }
    }
}

void parse_config_file(struct _config_db *config_db, struct experiment_config *exp_config) {
    FILE *file = fopen("config", "r");
    if (!file) {
        perror("Error opening file");
        return;
    }

    char line[200];
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "row_count")) {
            sscanf(line, "row_count = %u", &config_db->row_count);
        } else if (strstr(line, "num_columns")) {
            sscanf(line, "num_columns = %u", &config_db->num_columns);
            config_db->row_size = config_db->num_columns * COLUMN_WIDTH;
        } else if (strstr(line, "store_type")) {
            sscanf(line, "store_type = %c", &config_db->store_type);
        } else if (strstr(line, "column_type")) {
            sscanf(line, "column_type = %c", &config_db->col_type);
        } else if (strstr(line, "min")) {
            sscanf(line, "min = %u", &config_db->min);
        } else if (strstr(line, "max")) {
            sscanf(line, "max = %u", &config_db->max);
        } else if (strstr(line, "mvcc_enabled")) {
            char mvcc_str[6];
            sscanf(line, "mvcc_enabled = %5s", mvcc_str);
            config_db->mvcc_enabled = (strcmp(mvcc_str, "true") == 0);
        } else if (strstr(line, "print")) {
            char print_str[6];
            sscanf(line, "print = %5s", print_str);
            config_db->print = (strcmp(print_str, "true") == 0);
        } else if (strstr(line, "r_col")) {
            sscanf(line, "r_col = %u", &exp_config->r_col);
        } else if (strstr(line, "num_samples")) {
            sscanf(line, "num_samples = %u", &exp_config->num_samples);
        }
        // Add more settings here if needed
    }

    fclose(file);
}




