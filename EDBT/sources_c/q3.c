#define _GNU_SOURCE
#include "query_header.h"

int main(int argc, char** argv) {
    struct _config config;
    unsigned int cycleHi    = 0, cycleLo=0;
    T k;

    // -- pasring arguments -------------------------------------------
    int opt;
    while ((opt = getopt(argc, argv, "C:R:W:r:O:F:k:")) != -1) {
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
            case 'k': {
                k = (T)atoi(optarg); break;
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

    unsigned dram_size  = config.row_count*config.row_size;
    int hpm_fd          = open_fd();
    int dram_fd         = open_fd();

    //mapping fpga:
    T* plim = mmap((void*)0, RELCACHE_SIZE, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED|0x40, hpm_fd, RELCACHE_ADDR);
    //mapping dram
    T* dram = mmap((void*)0, dram_size+config.frame_offset, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED|0x40, dram_fd, DRAM_ADDR);

    T data;
    T sumq = 0;

    // assuming 1st column used for selection
    magic_timing_begin(&cycleLo, &cycleHi);
    for(int i = 0; i < config.row_count; i++){
      if (plim[i*config.enabled_col_num] < k)
      {
        sumq += plim[i*config.enabled_col_num + 1];
      }    
    }
    magic_timing_end(&cycleLo, &cycleHi);
    printf("q3, r, c, %d, %d, %d, %d\n", config.row_size, config.row_count, config.col_widths[0], cycleLo);

    sumq = 0;
    magic_timing_begin(&cycleLo, &cycleHi);
    for(int i = 0; i < config.row_count; i++){
      if (plim[i*config.enabled_col_num] < k)
      {
        sumq += plim[i*config.enabled_col_num + 1];
      }    
    }
    magic_timing_end(&cycleLo, &cycleHi);
    printf("q3, r, h, %d, %d, %d, %d\n", config.row_size, config.row_count, config.col_widths[0], cycleLo);

    sumq = 0;
    magic_timing_begin(&cycleLo, &cycleHi);
    for(int i = 0; i < config.row_count; i++){
      if (dram[(i*config.row_size)/sizeof(T)] < k)
      {
        sumq += dram[((i*config.row_size) + config.col_offsets[1])/sizeof(T)];
      }
    }
    magic_timing_end(&cycleLo, &cycleHi);

    printf("q3, d, -, %d, %d, %d, %d\n", config.row_size, config.row_count, config.col_widths[0], cycleLo);

    return 0;
}
