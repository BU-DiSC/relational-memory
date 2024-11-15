#include "exp_header.h"
#include "print_utils.h"
#include "parse_config.h"
#include "config.h"
#include <fcntl.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <sys/wait.h>
#include <sys/shm.h>

int open_fd() {
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd == -1) {
        printf("Can't open /dev/mem.\n");
        exit(0);
    }
    return fd;
}

FILE* open_file(const char *filename, const char *mode) {
    FILE *file = fopen(filename, mode);
    if (!file) {
        printf("%s\t: ", filename);
        perror("Error opening file");
        exit(EXIT_FAILURE); // or return NULL and handle it in the caller
    }
    return file;
}

void setup(int argc, char **argv, char *query_name, struct _config_db *config_db, struct experiment_config *exp_args, struct _config_query *config_query) {
    int query_type = query_name[1];

    set_config(query_type, config_db, exp_args, config_query);

    parse_args(argc, argv, config_db, exp_args, config_query);
    
    if (config_db->load_file == true && access("config", F_OK) != -1) {
        printf("Loading database configuration from the config file\n");
        parse_config_file(config_db, exp_args); // Load from the specified config file
    } else {
        fprintf(stderr, "'config' file not found. Using default configuration.\n");
    }
}

void run_single_query(struct _config_db *config_db, struct experiment_config *exp_config, struct _config_query* query_config,
                      QueryFunction query_func, char* filename) {

    FILE* fp = open_file(filename, "w");

    query_config->output_file = fp;
    fprintf(fp, "bench, mem, temp, enabled_col_num, row_size, row_count, col_width, cycles, l1_references, l1_refills, l2_references, l2_refills, inst_retired\n");

    config_db->column_widths = malloc(config_db->num_columns * sizeof(unsigned int));
    config_db->column_types = malloc(config_db->num_columns * sizeof(char));
    for (int i = 0; i < config_db->num_columns; i++) {
        config_db->column_widths[i] = exp_config->r_col;
        config_db->column_types[i] = config_db->col_type;
    }

    query_config->enabled_column_number = parse_exp_offsets(query_config); // set offsets in query_config and get enabled column number

    for (unsigned int sample = 1; sample <= exp_config->num_samples; sample++) {
        run_query(config_db, query_config, query_func);

        config_db->store_type = 'c';
        run_query(config_db, query_config, query_func);
    }
    
    free(config_db->column_widths);
    free(config_db->column_types);

    fclose(fp);
}

void run_query(struct _config_db *config_db, struct _config_query* query_config, QueryFunction query_func) {
    // temporary naive flush
    flush_cache();
    printf("DB generate\n");
    unsigned char* db = generate_db(*config_db);

#ifdef __aarch64__
    printf("config relmem\n");
    configure_relcache(*config_db, query_config);
#endif
    
    printf("run query\n");
    query_func(*config_db, *query_config, db);
    
    
#ifdef __aarch64__
    reset_relcache(0);
    // temporary naive flush
    flush_cache();
#endif
}

void flush_cache() {
    char *array = malloc(SIZE);

    if (array == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
    }
    memset(array, 0, SIZE);

    for (int i = 0; i < SIZE; ++i) {
        char value = array[i];
    }
    free(array);
}

int main(int argc, char **argv) {
    int opt;
    char *query_name = NULL;

    struct _config_query config_query;
    struct _config_db config_db;
    struct experiment_config exp_args;

    while ((opt = getopt(argc, argv, "r:c:q:phl")) != -1) {
        switch (opt) {
            case 'r': 
                query_name = optarg;
                char *filename = get_filename_from_query_type(query_name[1]);
                setup(argc, argv, query_name, &config_db, &exp_args, &config_query);
                
                QueryFunction query_func = get_query_function(query_name[1]);
                row_size_exp(&config_db, &exp_args, &config_query, query_func, filename);

                print_experiment_config(&exp_args, config_db, config_query);
                print_dotted_line(50);
                printf("Experiment results were saved in: %s\n\n", filename);
                break;
            case 'c': 
                query_name = optarg;
                filename = get_filename_from_query_type(query_name[1]);
                setup(argc, argv, query_name, &config_db, &exp_args, &config_query);
                
                query_func = get_query_function(query_name[1]);
                col_size_exp(&config_db, &exp_args, &config_query, query_func, filename);

                print_experiment_config(&exp_args, config_db, config_query);
                print_dotted_line(50);
                printf("Experiment results were saved in: %s\n\n", filename);
                break;
            case 'p':
                query_name = "q1";
                set_config(1, &config_db, &exp_args, &config_query);
                char *filename_proj = "data/result_projectivity.csv";
                QueryFunction query_func_proj = get_query_function(query_name[1]);
                projectivity_exp(&config_db, &exp_args, &config_query, query_func_proj, filename_proj);

                print_dotted_line(50);
                printf("Experiment results were saved in: %s\n\n", filename_proj);
                break;
            case 'q':
                query_name = optarg;
                char *filename_single = get_filename_from_query_type(query_name[1]);
                setup(argc, argv, query_name, &config_db, &exp_args, &config_query);
                
                QueryFunction query_func_single = get_query_function(query_name[1]);
                run_single_query(&config_db, &exp_args, &config_query, query_func_single, filename_single);

                printf("Performance results were saved in: %s\n", filename_single);
                break;
            case 'h':
                print_help(argv[0]);
                exit(EXIT_SUCCESS);
                break;
            default:
                fprintf(stderr, "Invalid option. Use -h for help.\n");
                exit(EXIT_FAILURE);
        }
    }

    if (argc == 1) {
        fprintf(stderr, "For usage information, use the -h option: %s -h\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    return 0;
}