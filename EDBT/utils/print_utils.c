#include "exp_header.h"
#include "print_utils.h"

void print_experiment_config(struct experiment_config *config, struct _config_db config_db, struct _config_query config_query) {
    printf("\nExperiment configuration\n");
    printf("\n");
    printf("number of experiments: %u\n", config->num_samples);
    printf("k value: %d\n", config_query.k_value);
    printf("enabled column number: %u\n", config_query.enabled_column_number);
    printf("column width: %u\n", config->r_col);
    printf("row sizes varying between %u and %u\n", 16, 256);
    printf("Row Count: %d\n", config_db.row_count);
    printf("Column Type: %c\n", config_db.col_type);
    printf("Min: %d\n", config_db.min);
    printf("Max: %d\n", config_db.max);
}

void print_config_info(struct _config *config) {
    printf("Configuration Values:\n");

    // Print unsigned ints
    printf("row_size: %u\n", config->row_size);
    printf("row_count: %u\n", config->row_count);
    printf("reset: %u\n", config->reset);
    printf("enabled_col_num: %u\n", config->enabled_col_num);
    
    // Print arrays of unsigned shorts
    printf("col_widths: ");
    for (int i = 0; i < MAX_GROUPS; i++) {
        printf("%hu ", config->col_widths[i]);
        if(i != MAX_GROUPS - 1) {
            printf(", ");
        }
    }
    printf("\n");

    printf("col_offsets: ");
    for (int i = 0; i < MAX_GROUPS; i++) {
        printf("%hu ", config->col_offsets[i]);
        if(i != MAX_GROUPS - 1) {
            printf(", ");
        }
    }
    printf("\n");

    // Print frame_offset
    printf("frame_offset: %u\n", config->frame_offset);
}

void print_cycles_from_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening the file");
        return;
    }

    char line[500];
    int line_count = 0;
    int cycles[3];

    while (fgets(line, sizeof(line), file)) {
        line_count++;
        if (line_count == 1) {  // Skip header
            continue;
        }

        // Tokenize the line by commas
        int token_count = 0;
        char *ptr = strtok(line, ",");
        while (ptr) {
            token_count++;
            if (token_count == 7) {  // 7th column is "cycles"
                cycles[line_count - 2] = atoi(ptr);
            }
            ptr = strtok(NULL, ",");
        }
    }
    printf("\nCPU cycles");
    printf("\nRME cold: %d\n", cycles[0]);
    printf("RME hot: %d\n", cycles[1]);
    printf("DRAM(row store): %d\n", cycles[2]);
    printf("\nThe complete performance results are saved in: %s\n\n", filename);
    fclose(file);
}

void print_query2_params(const struct _config_query *params) {
    printf("Enabled Column Number: %u\n", params->enabled_column_number);
    printf("Column Offsets: ");
    for (unsigned int i = 0; i < params->enabled_column_number; i++) {
        printf("%u ", params->col_offsets[i]);
    }
    printf("\nColumn Widths: ");
    for (unsigned int i = 0; i < params->enabled_column_number; i++) {
        printf("%u ", params->col_width[i]);
    }
    printf("\nk_value: %d\n", params->k_value);
    printf("Output File: %p\n", (void *)params->output_file);
}

const char* get_query_output_filename(char query_type) {
    static char filename[256];  // buffer to store the filename
    snprintf(filename, sizeof(filename), "data/q%d_query_out.csv", query_type - '0');
    return filename;
}

void print_dotted_line(int n) {
    char line[n + 1]; 
    memset(line, '.', n);
    line[n] = '\0';
    printf("%s\n", line);
}


void print_help(const char *program_name) {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "    Run row size experiment:\n");
    fprintf(stderr, "    %s -r <query_name> [options]\n", program_name);
    fprintf(stderr, "    e.g., %s -r q2 -N 1\n", program_name);
    
    fprintf(stderr, "    Run projectivity experiment:\n");
    fprintf(stderr, "    %s -p\n", program_name);
    
    fprintf(stderr, "    Run a single query mode:\n");
    fprintf(stderr, "    %s -q <query_name> [options]\n", program_name);
    fprintf(stderr, "    e.g., %s -q q2 -L -O 0,4 -K 136\n", program_name);
    fprintf(stderr, "    e.g., %s -q q2 -P -O 0,4 -K 136 -R 16 -T s\n", program_name);

    fprintf(stderr, "\nOptions to override the default database configuration:\n");
    fprintf(stderr, "  -L   Load database configuration from 'config' file\n");
    fprintf(stderr, "  -C   Number of columns (int, default: 8)\n");
    fprintf(stderr, "  -R   Number of row counts (int)\n");
    fprintf(stderr, "  -N   Number of experiment samples\n");
    fprintf(stderr, "  -O   Column offsets (comma-separated)\n");
    fprintf(stderr, "  -K   K value for query\n");
    fprintf(stderr, "  -S   Storage type: r (row store, default) | c (column store)\n");
    fprintf(stderr, "  -T   Column types: s (sorted) | r (random, default) | z (zero padded)\n");
    fprintf(stderr, "  -P   Print created DB and query results\n");
    fprintf(stderr, "  -m   Minimum value\n");
    fprintf(stderr, "  -M   Maximum value\n");
    fprintf(stderr, "  -V   Enable MVCC mode\n");

    fprintf(stderr, "\nFor more details, refer to the documentation.\n");
}



void print_db(struct _config_db config, unsigned char* db, unsigned int row_size) {
    int offset = 0;
    // print row store
    if (config.store_type == 'r') {
        for (int i = 0; i < config.row_count; i++) {
            offset = 0;
            for (int j = 0; j < config.num_columns; j++) {
                if (config.column_widths[j] == 1) {
                    unsigned char temp = *(unsigned char*)(db + (i * row_size) + offset);
                    printf("%d\t", temp);
                }
                else if (config.column_widths[j] == 2) {
                    unsigned short temp = *(unsigned short*)(db + (i * row_size) + offset);
                    printf("%d\t", temp);
                }
                else if (config.column_widths[j] == 4) {
                    unsigned int temp = *(unsigned int*)(db + (i * row_size) + offset);
                    printf("%d\t", temp);
                }
                else if (config.column_widths[j] == 8) {
                    unsigned long temp = *(unsigned long*)(db + (i * row_size) + offset);
                    printf("%ld\t", temp);
                }
                else {
                    for (int k = config.column_widths[j] - 1; k >= 0; k--) {
                        unsigned char temp = *(unsigned char*)(db + (i * row_size) + offset + k);
                        printf("%x", temp);
                    }
                    printf("\t");
                }
                offset += config.column_widths[j];
            }
            printf("\n");
        }
    }
    // print column store
    else {
        for (int i = 0; i < config.row_count; i++) {
            offset = 0;
            for (int j = 0; j < config.num_columns; j++) {
                if (config.column_widths[j] == 1) {
                    unsigned char temp = *(unsigned char*)(db + config.row_count * offset + i * config.column_widths[j]);
                    printf("%d\t", temp);
                }
                else if (config.column_widths[j] == 2) {
                    unsigned short temp = *(unsigned short*)(db + config.row_count * offset + i * config.column_widths[j]);
                    printf("%d\t", temp);
                }
                else if (config.column_widths[j] == 4) {
                    unsigned int temp = *(unsigned int*)(db + config.row_count * offset + i * config.column_widths[j]);
                    printf("%d\t", temp);
                }
                else if (config.column_widths[j] == 8) {
                    unsigned long temp = *(unsigned long*)(db + config.row_count * offset + i * config.column_widths[j]);
                    printf("%ld\t", temp);
                }
                else {
                    for (int k = config.column_widths[j] - 1; k >= 0; k--) {
                        unsigned char temp = *(unsigned char*)(db + config.row_count * offset + i * config.column_widths[j] + k);
                        printf("%x", temp);
                    }
                    printf("\t");
                }
                offset += config.column_widths[j];
            }
            printf("\n");
        }
    }
    return;
}