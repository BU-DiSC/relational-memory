#define _GNU_SOURCE
#include "exp_header.h"
#include "performance_counters.h"

void run_query1(struct _config_db config_db, struct _config_query params) {

    unsigned int cycleHi    = 0, cycleLo=0;
    struct perf_counters res, start, end;
    int fd = setup_pmcs();
    if (fd < 0)
        perror("Issue opening PMC FDs\n");

    bool mvcc_enabled = false;

    unsigned sum_col_width = 0;
    for ( int i=0 ; i<params.enabled_column_number ; i++ ) {
        sum_col_width += config_db.column_widths[i];
    }
    //-- pasring arguments done --------------------------------------

    unsigned dram_size  = config_db.row_count*config_db.row_size;
    int hpm_fd          = open_fd();
    int dram_fd         = open_fd();

    //mapping fpga:
    unsigned char* plim = mmap((void*)0, RELCACHE_SIZE, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED|0x40, hpm_fd, RELCACHE_ADDR);
    //mapping dram
    unsigned char* dram = mmap((void*)0, dram_size, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED|0x40, dram_fd, DRAM_ADDR);

    T data;

    if ( config_db.store_type == 'r' ){
        // move multiplication outside
        unsigned width = config_db.column_widths[0];
        pmcs_get_value(&start);
        magic_timing_begin(&cycleLo, &cycleHi);
        for(int i = 0; i < config_db.row_count; i++){
          // when the row is valid
          unsigned offset = 0;
          for(int j=0; j<params.enabled_column_number; j++){
            data = *(T*)(plim + i*sum_col_width + width*j);
          }    
        }
        magic_timing_end(&cycleLo, &cycleHi);
        pmcs_get_value(&end);
        res = pmcs_diff(&end, &start);
        fprintf(params.output_file,"q1, r, c, %d, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", params.enabled_column_number, config_db.row_size, config_db.row_count, config_db.column_widths[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);

        pmcs_get_value(&start);
        magic_timing_begin(&cycleLo, &cycleHi);
        for(int i = 0; i < config_db.row_count; i++){
          // when the row is valid
          unsigned offset = 0;
          for(int j=0; j<params.enabled_column_number; j++){
            data = *(T*)(plim + i*sum_col_width + width*j);
          }   
        }

        magic_timing_end(&cycleLo, &cycleHi);
        pmcs_get_value(&end);
        res = pmcs_diff(&end, &start);
        fprintf(params.output_file,"q1, r, h, %d, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", params.enabled_column_number, config_db.row_size, config_db.row_count, config_db.column_widths[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);

        pmcs_get_value(&start);
        magic_timing_begin(&cycleLo, &cycleHi);
        for(int i = 0; i < config_db.row_count; i++){
          // when the row is valid
          for(int j=0; j<params.enabled_column_number; j++){
              data = *(T*)(dram + i*config_db.row_size + params.col_offsets[j]);
          }
        }
        magic_timing_end(&cycleLo, &cycleHi);
        pmcs_get_value(&end);
        res = pmcs_diff(&end, &start);
       fprintf(params.output_file,"q1, d, -, %d, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", params.enabled_column_number, config_db.row_size, config_db.row_count, config_db.column_widths[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);
    }
    else if ( config_db.store_type == 'c' ){

        // columnar access
        unsigned char* result = (unsigned char *)malloc(params.enabled_column_number * sizeof(T));
        pmcs_get_value(&start);
        magic_timing_begin(&cycleLo, &cycleHi);
        for(int i = 0; i < config_db.row_count; i++)
        {
          if (params.enabled_column_number == 2){
            memcpy(result, (dram + i), sizeof(T));
            memcpy(result + 1, (dram + i + config_db.row_count), sizeof(T));
          }
          else if (params.enabled_column_number == 3){
            memcpy(result, (dram + i), sizeof(T));
            memcpy(result + 1, (dram + i + config_db.row_count), sizeof(T));
            memcpy(result + 2, (dram + i + 2*config_db.row_count), sizeof(T));
          }
          else if (params.enabled_column_number == 4){
            memcpy(result, (dram + i), sizeof(T));
            memcpy(result + 1, (dram + i + config_db.row_count), sizeof(T));
            memcpy(result + 2, (dram + i + 2*config_db.row_count), sizeof(T));
            memcpy(result + 3, (dram + i + 3*config_db.row_count), sizeof(T));
          }
          else if (params.enabled_column_number == 5){
            memcpy(result, (dram + i), sizeof(T));
            memcpy(result + 1, (dram + i + config_db.row_count), sizeof(T));
            memcpy(result + 2, (dram + i + 2*config_db.row_count), sizeof(T));
            memcpy(result + 3, (dram + i + 3*config_db.row_count), sizeof(T));
            memcpy(result + 4, (dram + i + 4*config_db.row_count), sizeof(T));
          }
          else if (params.enabled_column_number == 6){
            memcpy(result, (dram + i), sizeof(T));
            memcpy(result + 1, (dram + i + config_db.row_count), sizeof(T));
            memcpy(result + 2, (dram + i + 2*config_db.row_count), sizeof(T));
            memcpy(result + 3, (dram + i + 3*config_db.row_count), sizeof(T));
            memcpy(result + 4, (dram + i + 4*config_db.row_count), sizeof(T));
            memcpy(result + 5, (dram + i + 5*config_db.row_count), sizeof(T));
          }
          else if (params.enabled_column_number == 7){
            memcpy(result, (dram + i), sizeof(T));
            memcpy(result + 1, (dram + i + config_db.row_count), sizeof(T));
            memcpy(result + 2, (dram + i + 2*config_db.row_count), sizeof(T));
            memcpy(result + 3, (dram + i + 3*config_db.row_count), sizeof(T));
            memcpy(result + 4, (dram + i + 4*config_db.row_count), sizeof(T));
            memcpy(result + 5, (dram + i + 5*config_db.row_count), sizeof(T));
            memcpy(result + 6, (dram + i + 6*config_db.row_count), sizeof(T));
          }
          else if (params.enabled_column_number == 8){
            memcpy(result, (dram + i), sizeof(T));
            memcpy(result + 1, (dram + i + config_db.row_count), sizeof(T));
            memcpy(result + 2, (dram + i + 2*config_db.row_count), sizeof(T));
            memcpy(result + 3, (dram + i + 3*config_db.row_count), sizeof(T));
            memcpy(result + 4, (dram + i + 4*config_db.row_count), sizeof(T));
            memcpy(result + 5, (dram + i + 5*config_db.row_count), sizeof(T));
            memcpy(result + 6, (dram + i + 6*config_db.row_count), sizeof(T));
            memcpy(result + 7, (dram + i + 7*config_db.row_count), sizeof(T));
          }
          else if (params.enabled_column_number == 9){
            memcpy(result, (dram + i), sizeof(T));
            memcpy(result + 1, (dram + i + config_db.row_count), sizeof(T));
            memcpy(result + 2, (dram + i + 2*config_db.row_count), sizeof(T));
            memcpy(result + 3, (dram + i + 3*config_db.row_count), sizeof(T));
            memcpy(result + 4, (dram + i + 4*config_db.row_count), sizeof(T));
            memcpy(result + 5, (dram + i + 5*config_db.row_count), sizeof(T));
            memcpy(result + 6, (dram + i + 6*config_db.row_count), sizeof(T));
            memcpy(result + 7, (dram + i + 7*config_db.row_count), sizeof(T));
            memcpy(result + 8, (dram + i + 8*config_db.row_count), sizeof(T));
          }
          else if (params.enabled_column_number == 10){
            memcpy(result, (dram + i), sizeof(T));
            memcpy(result + 1, (dram + i + config_db.row_count), sizeof(T));
            memcpy(result + 2, (dram + i + 2*config_db.row_count), sizeof(T));
            memcpy(result + 3, (dram + i + 3*config_db.row_count), sizeof(T));
            memcpy(result + 4, (dram + i + 4*config_db.row_count), sizeof(T));
            memcpy(result + 5, (dram + i + 5*config_db.row_count), sizeof(T));
            memcpy(result + 6, (dram + i + 6*config_db.row_count), sizeof(T));
            memcpy(result + 7, (dram + i + 7*config_db.row_count), sizeof(T));
            memcpy(result + 8, (dram + i + 8*config_db.row_count), sizeof(T));
            memcpy(result + 9, (dram + i + 9*config_db.row_count), sizeof(T));
          }
          
        }    
        magic_timing_end(&cycleLo, &cycleHi);
        pmcs_get_value(&end);
        res = pmcs_diff(&end, &start);
        fprintf(params.output_file,"q1, c, -, %d, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", params.enabled_column_number, config_db.row_size, config_db.row_count, config_db.column_widths[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);
        free(result);
    }

    int ret = teardown_pmcs();
    if (ret < 0)
        perror("Issue detected while tearing down the PMCs\n");

    fflush(params.output_file);

    munmap(plim, RELCACHE_SIZE);
    munmap(dram, dram_size);

    close(hpm_fd);
    close(dram_fd);
}




























