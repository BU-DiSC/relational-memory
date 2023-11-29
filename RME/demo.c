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


void run_query(struct _config_db *config_db, struct _config_query* query_config, QueryFunction query_func) {
    // temporary naive flush
    flush_cache();
    generate_db(*config_db);
    printf("DB generate\n");

    configure_relcache(*config_db, query_config);
    printf("config relmem\n");

    query_func(*config_db, *query_config);
    printf("run query\n");
    
    reset_relcache(0);
    //call_db_reset_relcache(0);
    // temporary naive flush
    flush_cache();
}


void setup(int argc, char **argv, char *query_name, struct _config_db *config_db, struct experiment_config *exp_args, struct _config_query *config_query, char **filename) {
    int query_type = query_name[1];
    *filename = get_filename_from_query_type(query_type);

    ConfigFunction config_func = get_config_function(query_type);
    config_func(config_db, exp_args, config_query);

    if (access("config", F_OK) != -1) {
        parse_config_file(query_name); // Override with config file values
    } else {
        fprintf(stderr, "Warning: 'config' file not found. Using default configuration.\n");
    }

    parse_args(argc, argv, config_db, exp_args, config_query);
}


int main(int argc, char **argv) {
    int opt;
    char *query_name = NULL;

    struct _config_query config_query;
    struct _config_db config_db;
    struct experiment_config exp_args;

    while ((opt = getopt(argc, argv, "r:p:h")) != -1) {
        switch (opt) {
            case 'r': 
                query_name = optarg;
                char *filename;
                setup(argc, argv, query_name, &config_db, &exp_args, &config_query, &filename);
                
                QueryFunction query_func = get_query_function(query_name[1]);
                row_size_exp(&config_db, &exp_args, &config_query, query_func, filename);

                print_experiment_config(&exp_args, config_db, config_query);
                print_dotted_line(50);
                printf("Experiment results were saved in: %s\n\n", filename);
                break;
            case 'p':
                query_name = optarg;
                char *filename_proj;
                setup(argc, argv, query_name, &config_db, &exp_args, &config_query, &filename_proj);
                
                QueryFunction query_func_proj = get_query_function(query_name[1]);
                projectivity_exp(&config_db, &exp_args, &config_query, query_func_proj, filename_proj);

                print_dotted_line(50);
                printf("Experiment results were saved in: %s\n\n", filename_proj);
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










// int main(int argc, char **argv) {
//     int opt;
//     char *query_name = NULL;

//     struct _config_query config_query;
//     struct _config_db config_db;
//     struct experiment_config exp_args;
//     char *filename;
//     // Use getopt to process command-line arguments
//     while ((opt = getopt(argc, argv, "e:qh")) != -1) {
//         switch (opt) {
//             case 'e': 
//                 query_name = optarg;
//                 int query_type = query_name[1];
//                 filename = get_filename_from_query_type(query_type);

//                 // set hard coded default configuration for the experiment
//                 ConfigFunction config_func = get_config_function(query_type);
//                 config_func(&config_db, &exp_args, &config_query);

//                 // Check if the "config" file exists and override the default config
//                 if (access("config", F_OK) != -1) {
//                     // set the values from the config file at runtime
//                     parse_config_file(query_name);
//                 } else {
//                     fprintf(stderr, "Warning: 'config' file not found. Using default configuration.\n");
//                 }

//                 // override the previous config with command line arguments if any
//                 parse_args(argc, argv, &config_db, &exp_args, &config_query);

//                 // get the query function to run based on the name
//                 QueryFunction query_func = get_query_function(query_type);
//                 //run the experiment
//                 row_size_exp(&config_db, &exp_args,&config_query, query_func, filename);

//                 print_experiment_config(&exp_args, config_db, config_query);
//                 print_dotted_line(50);
//                 printf("Experiment results were saved in: %s\n\n", filename);

//                 break;
//             case 'h':
//                 print_help(argv[0]);
//                 exit(EXIT_SUCCESS); 
//         }
//     }

//     if (argc == 1) {
//         fprintf(stderr, "For usage information, use the -h option: %s -h\n", argv[0]);
//         exit(EXIT_FAILURE);
//     }

//     return 0;
// }