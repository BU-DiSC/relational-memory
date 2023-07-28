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

#define KB      1024
#define MB      KB*KB
#define WORD_SIZE 128


#define HIGH_DDR_ADDR 0x800000000

#define BUS_WIDTH      16

unsigned int get_uniform(unsigned int rangeLow, unsigned int rangeHigh);
int open_fd() {
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd == -1) {
        printf("Can't open /dev/mem.\n");
        exit(0);
    }
    return fd;
}

int main(int argc, char** argv) {

    unsigned row_size = 64;
    unsigned row_count = 32;
    unsigned num_columns = 8;
    char     store_type = 'r';
    bool     print = false;
    unsigned *column_widths = 0;
    char     *column_types = 0;
    //unsigned column_widths[64];
    //char     column_types[64];
    //for( int i=0 ; i<64; i++ ){
    //    column_widths[i] = 8;
    //    column_types[i]  = 'r';
    //}
    unsigned min = 0;
    unsigned max = 1000;
    
    // -- pasring arguments -------------------------------------------
    int opt;
    while ((opt = getopt(argc, argv, "C:R:S:W:T:Pr:m:M:")) != -1) {
        switch (opt) {
            case 'C': {
                num_columns = atoi(optarg); 
                column_widths = (unsigned int *) malloc ( num_columns * sizeof(unsigned int) );
                column_types = (char *) malloc ( num_columns * sizeof(char) );
                for( int i=0 ; i<64; i++ ){
                    column_widths[i] = 8;
                    column_types[i]  = 'r';
                }
                break;
            }
            case 'R': {
                row_count = atoi(optarg); break;
            }
            case 'r': {
                row_size = atoi(optarg); break;
            }
            case 'W': {
                char *pt;
                pt = strtok( optarg, "," );
                int i = 0;
                while (pt != NULL) {
                    //if ( i >= 64 ){
                    //    fprintf(stderr, "Usage: %s [-W] The maximun number of columns is 64.\n", argv[0]);
                    //    exit(EXIT_FAILURE);
                    //}
                    column_widths[i] = atoi(pt);
                    //printf("%d\n", column_width_temp[i]);
                    pt = strtok (NULL, ",");
                    i++;
                }
                //printf("%d", num_columns);
                break;
            }
            case 'T': {
                char *pt;
                pt = strtok( optarg, "," );
                int i = 0;
                while (pt != NULL) {
                    //if ( i >= 64 ){
                    //    fprintf(stderr, "Usage: %s [-T] The maximun number of columns is 64.\n", argv[0]);
                    //    exit(EXIT_FAILURE);
                    //}
                    column_types[i] = *pt;
                    if ( column_types[i] != 'r' && column_types[i] != 's' && column_types[i] != 'z' ){
                        fprintf(stderr, " Usage:   [-T] array of column types (s: sorted, r:random, z:zero padded) (default: r)\n");
                        exit(EXIT_FAILURE);
                    }
                    pt = strtok (NULL, ",");
                    i++;
                }
                break;
            }
            case 'S': {
                char type = *optarg;
                if ( type != 'r' && type != 'c' ){
                    fprintf(stderr, "Usage: %s [-S] r: row store, c: column store (default: r)\n", argv[0]);
                    exit(EXIT_FAILURE);
                }
                //printf("%s", optarg);
                store_type = type; break;
            }
            case 'P': {
                print = true; break;
            }
            case 'm': {
                min = (unsigned int)atoi(optarg); break;
            }
            case 'M': {
                max = (unsigned int)atoi(optarg); break;
            }
            default: {
                fprintf(stderr, "Usage: %s [-S] r: row store, c: column store (default: r)\n", argv[0]);
                fprintf(stderr, "          [-C] number of columns (int) (default: 8)\n");
                fprintf(stderr, "          [-r] row size (int) (default: 64)\n");
                fprintf(stderr, "          [-R] number of row counts (int)\n");
                fprintf(stderr, "          [-W] array of column widths (\"int, int, ..., int\") (default: 8)\n");
                fprintf(stderr, "          [-T] array of column types (s: sorted, r:random, z:zero padded) (default: r)\n");
                fprintf(stderr, "          [-P] print created DB\n");
                fprintf(stderr, "          [-m] min value for random type\n");
                fprintf(stderr, "          [-M] max value for random type\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    unsigned check_row_size = 0;
    for( int i=0 ; i<num_columns; i++ ){
        check_row_size += column_widths[i];
    }
    row_size = check_row_size;
    // -- pasring arguments done --------------------------------------

    unsigned db_size = row_count*row_size;
        
    #ifdef linux
        unsigned char* db = (unsigned char *) malloc ( db_size * sizeof(unsigned char) );
    #else
        int hpm_fd = open_fd();
        unsigned char* db = mmap((void*)0, db_size, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED, hpm_fd, HIGH_DDR_ADDR); //Uncached mapping
    #endif
   
    //Presetting the DB region to 0
    for(int i=0; i<db_size; i++){
        db[i] = 0;            
    }

    // building DB
    __uint128_t    value = 0;
    int offset = 0;
    for(int j=0; j<num_columns; j++){
        for(int i=0; i<row_count; i++){
            // pick column value according to the type
            if ( column_types[j] == 's' ) {
                value = (__uint128_t)i;
            }
            else if ( column_types[j] == 'r' ) {
                value = get_uniform(min, max);
            }
            else if ( column_types[j] == 'z' ) {
                value = 0;
            }
            //printf("%d %d %ld\n", j, i, (unsigned long)value);


            // row store
            if ( store_type == 'r' ) {
                //printf("offset %d \n", offset);
                if ( column_widths[j]==1 ) {
    		        *(unsigned char*)( db + (i*row_size) + offset) = (unsigned char)value;
                }
                else if ( column_widths[j]==2 ) {
    		        *(unsigned short*)(db + (i*row_size) + offset) = (unsigned short)value;
                }
                else if ( column_widths[j]==4 ) {
    		        *(unsigned int*)(  db + (i*row_size) + offset) = (unsigned int)value;
                }
                else if ( column_widths[j]==8 ) {
    		        *(unsigned long*)( db + (i*row_size) + offset) = (unsigned long)value;
                }
                else{
                    for ( int k=0 ; k<column_widths[j] ; k++ ) {
    		            *(unsigned char*)(db + (i*row_size) + offset + k) = (unsigned char)value;
                        if ( column_types[j] == 's' ) {
                            value = value/64;
                        }
                    }
                }
            }
            // column store
            else if ( store_type == 'c' ){
                if ( column_widths[j]==1 ) {
    		        *(unsigned char*)(db + row_count*offset + i*column_widths[j]) = (unsigned char)value;
                }
                else if ( column_widths[j]==2 ) {
    		        *(unsigned short*)(db + row_count*offset + i*column_widths[j]) = (unsigned short)value;
                }
                else if ( column_widths[j]==4 ) {
    		        *(unsigned int*)(db + row_count*offset + i*column_widths[j]) = (unsigned int)value;
                }
                else if ( column_widths[j]==8 ) {
    		        *(unsigned long*)(db + row_count*offset + i*column_widths[j]) = (unsigned long)value;
                }
                else{
                    for ( int k=0 ; k<column_widths[j] ; k++ ) {
    		            *(unsigned char*)(db + row_count*offset + i*column_widths[j] + k) = (unsigned char)value;
                        if ( column_types[j] == 's' ) {
                            value = value/64;
                        }
                    }
                }
            }
    	}
        offset += column_widths[j];
    }
    printf("DB is built\n");

    // print DB
    if ( print==true) {
        if ( store_type == 'r' ) {
            for(int i=0; i<row_count; i++){
                offset = 0;
                for(int j=0; j<num_columns; j++){
                    if ( column_widths[j]==1 ) {
        		        unsigned char temp = *( unsigned char*)(db + (i*row_size) + offset);
                        printf("%d\t", temp);
                    }
                    else if ( column_widths[j]==2 ) {
        		        unsigned short temp = *(  unsigned short*)(db + (i*row_size) + offset);
                        printf("%d\t", temp);
                    }
                    else if ( column_widths[j]==4 ) {
        		        unsigned int temp = *(  unsigned int*)(db + (i*row_size) + offset);
                        printf("%d\t", temp);
                    }
                    else if ( column_widths[j]==8 ) {
        		        unsigned long temp = *(unsigned long*)(db + (i*row_size) + offset);
                        printf("%ld\t", temp);
                    }
                    else{
                        for ( int k=column_widths[j]-1 ; k>=0 ; k-- ) {
    		                unsigned char temp = *(unsigned char*)(db + (i*row_size) + offset + k);
                            printf("%x", temp);
                        }
                        printf("\t");
                    }
                    offset += column_widths[j];
                }
                printf("\n");
            }
        }
        // print column store
        else {
            for(int i=0; i<row_count; i++){
                offset = 0;
                for(int j=0; j<num_columns; j++){
                    if ( column_widths[j]==1 ) {
        		        unsigned char temp = *(unsigned char*)(db + row_count*offset + i*column_widths[j]);
                        printf("%d\t", temp);
                    }
                    else if ( column_widths[j]==2 ) {
        		        unsigned short temp = *(unsigned short*)(db + row_count*offset + i*column_widths[j]);
                        printf("%d\t", temp);
                    }
                    else if ( column_widths[j]==4 ) {
        		        unsigned int temp = *(  unsigned int*)(db + row_count*offset + i*column_widths[j]);
                        printf("%d\t", temp);
                    }
                    else if ( column_widths[j]==8 ) {
        		        unsigned long temp = *(unsigned long*)(db + row_count*offset + i*column_widths[j]);
                        printf("%ld\t", temp);
                    }
                    else{
                        //for ( int k=0 ; k<column_widths[j] ; k++ ) {
                        for ( int k=column_widths[j]-1 ; k>=0 ; k-- ) {
    		                unsigned char temp = *(unsigned char*)(db + row_count*offset + i*column_widths[j] + k);
                            printf("%x", temp);
                        }
                        printf("\t");
                    }
                    offset += column_widths[j];
                }
                printf("\n");
            }
        }
    }

    return 0;
}

unsigned int get_uniform(unsigned int rangeLow, unsigned int rangeHigh) {
    double myRand = rand()/(1.0 + RAND_MAX); 
    unsigned int range = rangeHigh - rangeLow + 1;
    unsigned int myRand_scaled = (myRand * range) + rangeLow;
    return myRand_scaled;
}
