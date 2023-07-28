#define _GNU_SOURCE
#include "query_header.h"
#include "../include/performance_counters.h"

int main(int argc, char** argv) {
    struct _config config;
    unsigned int cycleHi    = 0, cycleLo=0;
    struct perf_counters res, start, end;
    char store_type = 'r';
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
            case 'S': {
                char type = *optarg;
                if ( type != 'r' && type != 'c' ){
                    fprintf(stderr, "Usage: %s [-S] r: row store, c: column store (default: r)\n", argv[0]);
                    exit(EXIT_FAILURE);
                }
                //printf("%s", optarg);
                store_type = type; break;
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

    int fd = setup_pmcs();
    if (fd < 0)
        perror("Issue opening PMC FDs\n");

    //mapping fpga:
    T* plim = mmap((void*)0, RELCACHE_SIZE, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED|0x40, hpm_fd, RELCACHE_ADDR);
    //mapping dram
    T* dram = mmap((void*)0, dram_size+config.frame_offset, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED|0x40, dram_fd, DRAM_ADDR);

    T data;

    if ( store_type == 'r' ){
        // assuming 2nd column used for selection
        pmcs_get_value(&start);
        magic_timing_begin(&cycleLo, &cycleHi);
        for(int i = 0; i < config.row_count; i++){
          if (plim[i*config.enabled_col_num + 1] > k)
          {
            data = plim[i*config.enabled_col_num];
          }
        }
        magic_timing_end(&cycleLo, &cycleHi);
        pmcs_get_value(&end);
        res = pmcs_diff(&end, &start);
        printf("q2_new, r, c, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", config.row_size, config.row_count, config.col_widths[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);
    
    
        pmcs_get_value(&start);
        magic_timing_begin(&cycleLo, &cycleHi);
        for(int i = 0; i < config.row_count; i++){
          if (plim[i*config.enabled_col_num + 1] > k)
          {
            data = plim[i*config.enabled_col_num];
          }
        }
        magic_timing_end(&cycleLo, &cycleHi);
        pmcs_get_value(&end);
        res = pmcs_diff(&end, &start);
        printf("q2_new, r, h, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", config.row_size, config.row_count, config.col_widths[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);
    
    
        pmcs_get_value(&start);
        magic_timing_begin(&cycleLo, &cycleHi);
        for(int i = 0; i < config.row_count; i++){
          if (dram[(i*config.row_size + config.col_offsets[1])/sizeof(T)] > k)
          {
            data = dram[(i*config.row_size)/sizeof(T)];
          }
        }
        magic_timing_end(&cycleLo, &cycleHi);
        pmcs_get_value(&end);
        res = pmcs_diff(&end, &start);
        printf("q2_new, d, -, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", config.row_size, config.row_count, config.col_widths[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);
    }


    if ( store_type == 'c' ){
    	pmcs_get_value(&start);
    	magic_timing_begin(&cycleLo, &cycleHi);
    	for(int i = 0; i < config.row_count; i++)
    	{
    	  if (dram[i + config.row_count * (config.col_offsets[1]/sizeof(T))] > k)
    	  {
    	    data = dram[i];
    	  }
    	}
    	magic_timing_end(&cycleLo, &cycleHi);
    	pmcs_get_value(&end);
    	res = pmcs_diff(&end, &start);
    	printf("q2_new, c, -, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", config.row_size, config.row_count, config.col_widths[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);
    }

    int ret = teardown_pmcs();
    if (ret < 0)
        perror("Issue detected while tearing down the PMCs\n");

    return 0;
}
