#define _GNU_SOURCE
#include "query_header.h"

int main(int argc, char** argv) {
    struct _config config;
    unsigned int cycleHi=0, cycleLo=0;
  
    // -- pasring arguments -------------------------------------------
    int opt;
    while ((opt = getopt(argc, argv, "C:R:W:r:O:F:")) != -1) {
        switch (opt) {
            case 'C': {
                config.enabled_col_num = atoi(optarg); break;
            }
            case 'R': {
                config.row_count = atoi(optarg); break;
            }
            case 'r': {
                config.row_size = atoi(optarg); break;
            }
            case 'F': {
                config.frame_offset = atoi(optarg); break;
            }
            case 'W': {
                char *pt;
                pt = strtok( optarg, "," );
                int i = 0;
                while (pt != NULL) {
                    if ( i >= 11 ){
                        fprintf(stderr, "Usage: [-W] The maximun number of columns is 11.\n");
                        exit(EXIT_FAILURE);
                    }
                    config.col_widths[i] = atoi(pt);
                    pt = strtok (NULL, ",");
                    i++;
                }
                break;
            }
            case 'O': {
                char *pt;
                pt = strtok( optarg, "," );
                int i = 0;
                while (pt != NULL) {
                    if ( i >= 11 ){
                        fprintf(stderr, "Usage: [-O] The maximun number of columns is 11.\n");
                        exit(EXIT_FAILURE);
                    }
                    config.col_offsets[i] = atoi(pt);
                    if ( i > 0 ){
                        if ( config.col_offsets[i] <= config.col_offsets[i-1]){
                            fprintf(stderr, "Usage: [-O] The maximun number of columns is 11.\n");
                            exit(EXIT_FAILURE);
                        }
                    }
                    pt = strtok (NULL, ",");
                    i++;
                }
                break;
            }
            default: {
                fprintf(stderr, "Usage: %s [-C] number of columns (int) (default: 8)\n", argv[0]);
                fprintf(stderr, "          [-r] row size (int) (default: 64)\n");
                fprintf(stderr, "          [-R] number of row counts (int)\n");
                fprintf(stderr, "          [-W] array of column widths (\"int, int, ..., int\") \n");
                fprintf(stderr, "          [-O] array of column offsets (\"int, int, ..., int\") \n");
                exit(EXIT_FAILURE);
            }
        }
    }
    // -- pasring arguments done --------------------------------------
    unsigned int total_width = 0;;
    for( int i=0 ; i<config.enabled_col_num ; i++ ){
      total_width += config.col_widths[i];
    }
    __uint64_t data;

    //mapping dram
    unsigned dram_size  = config.row_count*config.row_size;
    int mem_fd  = open_fd();
    unsigned char* dram = mmap((void*)0, dram_size+config.frame_offset, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED|0x40, mem_fd, DRAM_ADDR);

    printf( "row size: %d\n", config.row_size );
    printf( "number of columns: %d\n", config.enabled_col_num );
    for (int j = 0; j < config.enabled_col_num; j++) {
      printf( "width of %d-th columns: %d %d\n", j, config.col_widths[j], config.col_offsets[j]);
    }

    printf( "Dumping row-store content:\n" );
    for(int i = 0; i < config.row_count; i++){
      for (int j = 0; j < config.enabled_col_num; j++) {
        if (config.col_widths[j] == 1 ){
    	    __uint8_t tmp = *(__uint8_t *)(dram + config.row_size*i + config.col_offsets[j]);
          printf("%u\t", tmp);
        }
        else if (config.col_widths[j] == 2 ){
    	    __uint16_t tmp = *(__uint16_t *)(dram + config.row_size*i + config.col_offsets[j]);
          printf("%u\t", tmp);
        }
        else if (config.col_widths[j] == 4 ){
    	    __uint32_t tmp = *(__uint32_t *)(dram + config.row_size*i + config.col_offsets[j]);
          printf("%u\t", tmp);
        }
        else if (config.col_widths[j] == 8 ){
    	    __uint64_t tmp = *(__uint64_t *)(dram + config.row_size*i + config.col_offsets[j]);
          printf("%lu\t", tmp);
        }
      }
      printf("\n");
    }
    
    //mapping fpga:
    int hpm_fd  = open_fd();
    unsigned char* plim = mmap((void*)0, RELCACHE_SIZE, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED|0x40, hpm_fd, RELCACHE_ADDR);

    printf( "Dumping Relcache data bram content:\n" );
    for(int i = 0; i < config.row_count; i++){
      int offset = 0;
      for (int j = 0; j < config.enabled_col_num; j++) {
        if (config.col_widths[j] == 1 ){
    	    __uint8_t tmp = *(__uint8_t *)(plim + total_width*i + offset);
          printf("%u\t", tmp);
        }
        else if (config.col_widths[j] == 2 ){
    	    __uint16_t tmp = *(__uint16_t *)(plim + total_width*i + offset);
          printf("%u\t", tmp);
        }
        else if (config.col_widths[j] == 4 ){
      	  __uint32_t tmp = *(__uint32_t *)(plim + total_width*i + offset);
          printf("%u\t", tmp);
        }
        else if (config.col_widths[j] == 8 ){
      	  __uint64_t tmp = *(__uint64_t *)(plim + total_width*i + offset);
          printf("%lu\t", tmp);
        }
        offset += config.col_widths[j];
      }
      printf("\n");
    }

    
    return 0;
}
