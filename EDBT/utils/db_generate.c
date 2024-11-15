#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include "config.h"
#include "print_utils.h"

// #define KB      1024
// #define MB      KB*KB
// #define WORD_SIZE 128


#define HIGH_DDR_ADDR 0x800000000

#define BUS_WIDTH      16

unsigned int get_uniform(unsigned int rangeLow, unsigned int rangeHigh) {
    double myRand = rand() / (1.0 + RAND_MAX);
    unsigned int range = rangeHigh - rangeLow + 1;
    unsigned int myRand_scaled = (myRand * range) + rangeLow;
    return myRand_scaled;
}

unsigned char * generate_db(struct _config_db config) {
    srand(1);
    unsigned check_row_size = 0;

    // Use 'config.num_columns' and 'config.column_widths'
    for (int i = 0; i < config.num_columns; i++) {
        check_row_size += config.column_widths[i];
    }

    unsigned row_size = check_row_size;

    // Use 'config.row_count' instead of 'row_count'
    unsigned db_size = config.row_count * row_size;
        
#ifdef __aarch64__
    int hpm_fd = open_fd();
    unsigned char* db = mmap((void*)0, db_size, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED, hpm_fd, HIGH_DDR_ADDR); //Uncached mapping
#else
    unsigned char* db = (unsigned char *) malloc ( db_size * sizeof(unsigned char) );
#endif

    for (int i = 0; i < db_size; i++) {
        db[i] = 0;            
    }

    __uint128_t value = 0;
    int offset = 0;


    // Use 'config.num_columns'
    for (int j = 0; j < config.num_columns; j++) {

        // Use 'config.row_count'
        for (int i = 0; i < config.row_count; i++) {
            if (config.column_types[j] == 's') {
                value = (__uint128_t)i;
            }
            else if (config.column_types[j] == 'r') {
                value = get_uniform(config.min, config.max);
            }
            else if (config.column_types[j] == 'z') {
                value = 0;
            }

            // Row Store
            if (config.store_type == 'r') {
                // Use 'config.column_widths'
                if (config.column_widths[j] == 1) {
                    *(unsigned char*)(db + (i * row_size) + offset) = (unsigned char)value;
                    
                }
                else if (config.column_widths[j] == 2) {
                  
                    *(unsigned short*)(db + (i * row_size) + offset) = (unsigned short)value;
                   
                }
                else if (config.column_widths[j] == 4) {
           
                    *(unsigned int*)(db + (i * row_size) + offset) = (unsigned int)value;
                    
                }
                else if (config.column_widths[j] == 8) {
    		        *(unsigned long*)( db + (i*row_size) + offset) = (unsigned long)value;
                }
                else {
                    for (int k = 0; k < config.column_widths[j]; k++) {
                       
                        *(unsigned char*)(db + (i * row_size) + offset + k) = (unsigned char)value;
                        if (config.column_types[j] == 's') {
                            value = value / 64;
                        }
                        
                    }
                }
            }
            // Column Store
            else if (config.store_type == 'c') {
                if (config.column_widths[j] == 1) {
                    *(unsigned char*)(db + config.row_count * offset + i * config.column_widths[j]) = (unsigned char)value;
                }
                else if (config.column_widths[j] == 2) {
                    *(unsigned short*)(db + config.row_count * offset + i * config.column_widths[j]) = (unsigned short)value;
                }
                else if (config.column_widths[j] == 4) {
                    *(unsigned int*)(db + config.row_count * offset + i * config.column_widths[j]) = (unsigned int)value;
                }
                else if (config.column_widths[j] == 8) {
                    *(unsigned long*)(db + config.row_count * offset + i * config.column_widths[j]) = (unsigned long)value;
                }
                else {
                    for (int k = 0; k < config.column_widths[j]; k++) {
                        *(unsigned char*)(db + config.row_count * offset + i * config.column_widths[j] + k) = (unsigned char)value;
                        if (config.column_types[j] == 's') {
                            value = value / 64;
                        }
                    }
                }
            }
        }
        offset += config.column_widths[j];


    }

    // print DB
    if (config.print == true) {
        print_db(config, db, row_size);
    }
#ifdef __aarch64__
    if (munmap(db, db_size) == -1) {
    perror("Error unmapping the memory");
    // Handle the error as appropriate
    }
#endif

    return db;

}









