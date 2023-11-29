#include "parse_config.h"
#include "exp_header.h"


void parse_exp_offsets(struct _config_query* query_config) {
    char *temp = strdup(query_config->offsets);  // Duplicate the string because strtok modifies the input
    char *token = strtok(temp, ",");
    int index = 0;
    while (token != NULL && index < MAX_GROUPS) {
        query_config->col_offsets[index] = (unsigned short) atoi(token);
        token = strtok(NULL, ",");
        index++;
    }
    free(temp);  
}

void populate_column_widths(struct _config_db* config, struct _config_query* q2_params){
    // Using the number of enabled columns instead of the full size
    for (int i = 0; i < q2_params->enabled_column_number; i++) {
        q2_params->col_width[i] = config->column_widths[i];
    }
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

ConfigFunction get_config_function(int query_type) {
    switch(query_type) {
        case '1':
            return set_q1_exp_default_config;
        case '2':
            return set_q2_exp_default_config;
        case '3':
            return set_q3_exp_default_config;
        case '4':
            return set_q4_exp_default_config;
        default:
            return NULL;
    }
}

char* get_filename_from_query_type(char query_type) {
    static char filename[50]; // Static because we're returning the local variable's address
    snprintf(filename, sizeof(filename), "data/PLT2_result_q%c_col.csv", query_type);
    return filename;
}

void parse_args(int argc, char **argv, struct _config_db *config_db, struct experiment_config *exp_config, struct _config_query *query_config) {
    int opt;
    while ((opt = getopt(argc, argv, "C:R:S:T:Pr:m:M:V:O:L:l:K:N:E:")) != -1) {
        switch (opt) {
            case 'C': {                
                config_db->num_columns = atoi(optarg);
                break;
            }
            case 'R': {
                config_db->row_count = atoi(optarg);
                break;
            }
            case 'r': {
                config_db->row_size = atoi(optarg);
                break;
            }
            case 'T': {
                char *pt;
                pt = strtok(optarg, ",");
                int i = 0;
                while (pt != NULL) {
                    config_db->column_types[i] = *pt;
                    if (config_db->column_types[i] != 'r' && config_db->column_types[i] != 's' && config_db->column_types[i] != 'z') {
                        fprintf(stderr, "Usage: [-T] array of column types (s: sorted, r:random, z:zero padded)\n");
                        exit(EXIT_FAILURE);
                    }
                    pt = strtok(NULL, ",");
                    i++;
                }
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
            case 'E': {                
                query_config->enabled_column_number = atoi(optarg);
                break;
            }
            case 'O': { // Assuming 'O' is for col_offsets
                char *token;
                int index = 0;
                token = strtok(optarg, ",");
                while(token != NULL && index < MAX_GROUPS) {
                    query_config->col_offsets[index] = atoi(token);
                    token = strtok(NULL, ",");
                    index++;
                }
                break;
            }
            case 'K': { // Assuming 'K' is for k_value
                query_config->k_value = atoi(optarg);
                break;
            }
        }
    }
}

void parse_config_file(const char* query_name) {
    char section[10] = {0};
    struct _config_db config_db;
    struct experiment_config exp_config;
    struct _config_query query_config;

    FILE *file = fopen("config", "r");
    if (!file) {
        perror("Error opening file");
        return;
    }

    char line[200];  
    bool is_query_section = false;  // a flag to know if we are inside the desired section

    while (fgets(line, sizeof(line), file)) {
        // Remove trailing newline
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
        }

        if (line[0] == '[') {
            if (is_query_section) {
                // if we already found our section and encounter another section, break
                break;
            }
            sscanf(line, "[%[^]]", section);
            if (strcmp(section, query_name) == 0) {
                is_query_section = true;
            }
        } else if (is_query_section) {
            // Parse data only if we are inside the desired query section
            if (strstr(line, "r_col")) {
                sscanf(line, "r_col = %u", &exp_config.r_col);
            } else if (strstr(line, "num_samples")) {
                sscanf(line, "num_samples = %u", &exp_config.num_samples);
            } else if (strstr(line, "offsets")) {
                sscanf(line, "offsets = \"%99[^\"]", query_config.offsets);
            } else if (strstr(line, "enabled_column_number")) {
                sscanf(line, "enabled_column_number = %u", &query_config.enabled_column_number);
            } else if (strstr(line, "k_value")) {
                sscanf(line, "k_value = %d", &query_config.k_value);
            } else if (strstr(line, "row_count")) {
                sscanf(line, "row_count = %u", &config_db.row_count);
            } else if (strstr(line, "store_type")) {
                sscanf(line, "store_type = '%c'", &config_db.store_type);
            } else if (strstr(line, "print")) {
                char tmp[6];
                sscanf(line, "print = %5s", tmp);
                config_db.print = (strcmp(tmp, "false") == 0) ? false : true;
            } else if (strstr(line, "min")) {
                sscanf(line, "min = %u", &config_db.min);
            } else if (strstr(line, "max")) {
                sscanf(line, "max = %u", &config_db.max);
            } else if (strstr(line, "mvcc_enabled")) {
                char tmp[6];
                sscanf(line, "mvcc_enabled = %5s", tmp);
                config_db.mvcc_enabled = (strcmp(tmp, "false") == 0) ? false : true;
            }
        }
    }

    fclose(file);

    if (!is_query_section) {
        fprintf(stderr, "Error: query section [%s] not found in the config file.\n", query_name);
    }
}




