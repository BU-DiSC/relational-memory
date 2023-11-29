#define _GNU_SOURCE
#include "exp_header.h"
#include "performance_counters.h"

void run_query1(struct _config_db config_db, struct _config_query params){
    
    unsigned int cycleHi    = 0, cycleLo=0;
    struct perf_counters res, start, end;

    unsigned dram_size  = config_db.row_count*config_db.row_size;
    int hpm_fd          = open_fd();
    int dram_fd         = open_fd();

    int fd = setup_pmcs();
    if (fd < 0)
        perror("Issue opening PMC FDs\n");

    //mapping fpga:
    T* plim = mmap((void*)0, RELCACHE_SIZE, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED|0x40, hpm_fd, RELCACHE_ADDR);
    //mapping dram
    T* dram = mmap((void*)0, dram_size, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED|0x40, dram_fd, DRAM_ADDR);

    T data;

    unsigned int *offset = malloc(params.enabled_column_number * sizeof(unsigned int));
    for (int j = 0; j < params.enabled_column_number; j++) {
        offset[j] = params.col_offsets[j] / sizeof(T);
    }

    T *cold_array = malloc(dram_size);
    T *hot_array = malloc(dram_size);
    T *row_array = malloc(dram_size);
    T *col_array = malloc(dram_size);
    T data_count = 0;
    if ( config_db.store_type == 'r' ){
        // assuming 2nd column used for selection
        pmcs_get_value(&start);
        magic_timing_begin(&cycleLo, &cycleHi);
        for(int i = 0; i < config_db.row_count; i++){
            for (int j = 0; j < params.enabled_column_number; j++){

                cold_array[data_count++] = plim[i*params.enabled_column_number + j];
            }    
        }
        magic_timing_end(&cycleLo, &cycleHi);
        pmcs_get_value(&end);
        res = pmcs_diff(&end, &start);
        fprintf(params.output_file,"q1, r, c, %d, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", config_db.row_size, params.enabled_column_number,config_db.row_count, params.col_width[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);
    
        data_count = 0;
        pmcs_get_value(&start);
        magic_timing_begin(&cycleLo, &cycleHi);
        for(int i = 0; i < config_db.row_count; i++){
            for (int j = 0; j < params.enabled_column_number; j++){

                hot_array[data_count++] = plim[i*params.enabled_column_number + j];
            }    
        }
        magic_timing_end(&cycleLo, &cycleHi);
        pmcs_get_value(&end);
        res = pmcs_diff(&end, &start);
        fprintf(params.output_file,"q1, r, h, %d, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", config_db.row_size, params.enabled_column_number,config_db.row_count, params.col_width[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);
    
        data_count = 0;
        pmcs_get_value(&start);
        magic_timing_begin(&cycleLo, &cycleHi);
        for(int i = 0; i < config_db.row_count; i++){
            for(int j=0; j< params.enabled_column_number; j++){

                row_array[data_count++] = dram[i * config_db.num_columns + offset[j]];
            }
        }
        magic_timing_end(&cycleLo, &cycleHi);
        pmcs_get_value(&end);
        res = pmcs_diff(&end, &start);
        fprintf(params.output_file,"q1, d, -, %d, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", config_db.row_size, params.enabled_column_number,config_db.row_count, params.col_width[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);
        // for (unsigned int i = 0; i < data_count; i++) {
        //     printf("q1, %d, %d, %d, %d\n", config_db.row_size, cold_array[i], hot_array[i], row_array[i]);
        // }
    }

    if ( config_db.store_type == 'c' ){
        data_count = 0;
        pmcs_get_value(&start);
        magic_timing_begin(&cycleLo, &cycleHi);
        for(int j = 0; j < params.enabled_column_number; j++){
            for(int i = 0; i < config_db.row_count; i++){

                col_array[data_count++] = dram[i + config_db.row_count * offset[j]];
            }
        }
        magic_timing_end(&cycleLo, &cycleHi);
        pmcs_get_value(&end);
        res = pmcs_diff(&end, &start);
        fprintf(params.output_file,"q1, c, -, %d, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", config_db.row_size, params.enabled_column_number,config_db.row_count, params.col_width[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);
    }
    int ret = teardown_pmcs();
    if (ret < 0)
        perror("Issue detected while tearing down the PMCs\n");

    fflush(params.output_file);

    munmap(plim, RELCACHE_SIZE);
    munmap(dram, dram_size);

    free(cold_array);
    free(hot_array);
    free(row_array);
    free(col_array);

    close(hpm_fd);
    close(dram_fd);
}