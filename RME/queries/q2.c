#define _GNU_SOURCE
#include "exp_header.h"
#include "performance_counters.h"

void run_query2(struct _config_db config_db, struct _config_query params){    

    unsigned int cycleHi    = 0, cycleLo=0;
    struct perf_counters res, start, end;

    unsigned int column0_offset = params.col_offsets[0]/sizeof(T);
    unsigned int column1_offset = params.col_offsets[1]/sizeof(T);

    unsigned dram_size  = config_db.row_count*config_db.row_size;
    int hpm_fd          = open_fd();
    int dram_fd         = open_fd();

    int fd = setup_pmcs();
    if (fd < 0)
        perror("Issue opening PMC FDs\n");

    //mapping fpga:
    T* plim = mmap((void*)0, RELCACHE_SIZE, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED|0x40, hpm_fd, RELCACHE_ADDR);
    //mapping dram
    T* dram = mmap((void*)0, dram_size, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED|0x40, hpm_fd, DRAM_ADDR);
    
    T data;

    T *cold_array = malloc(config_db.row_count * sizeof(T));
    T *hot_array = malloc(config_db.row_count * sizeof(T));
    T *row_array = malloc(config_db.row_count * sizeof(T));
    T *col_array = malloc(config_db.row_count * sizeof(T));
    // Add checks for successful memory allocation

    unsigned int data_count = 0;

    if ( config_db.store_type == 'r' ){
        pmcs_get_value(&start);
        magic_timing_begin(&cycleLo, &cycleHi);
        for (int i = 0; i < config_db.row_count; i++) {
            if (plim[i * params.enabled_column_number + 1] > params.k_value) {
                cold_array[data_count] = plim[i * params.enabled_column_number];
                data_count++;
            }
        }
        magic_timing_end(&cycleLo, &cycleHi);
        pmcs_get_value(&end);
        res = pmcs_diff(&end, &start);
        fprintf(params.output_file,"q2, r, c, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", config_db.row_size, config_db.row_count, params.col_width[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);
        
        data_count = 0;
        pmcs_get_value(&start);
        magic_timing_begin(&cycleLo, &cycleHi);
        for (int i = 0; i < config_db.row_count; i++) {
            if (plim[i * params.enabled_column_number + 1] > params.k_value) {
                hot_array[data_count]  = plim[i * params.enabled_column_number];
                data_count++;
            }
        }
        magic_timing_end(&cycleLo, &cycleHi);
        pmcs_get_value(&end);
        res = pmcs_diff(&end, &start);
        fprintf(params.output_file,"q2, r, h, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", config_db.row_size, config_db.row_count, params.col_width[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);
        
        data_count = 0;
        pmcs_get_value(&start);
        magic_timing_begin(&cycleLo, &cycleHi);
        for (int i = 0; i < config_db.row_count; i++) {
            if (dram[i * config_db.num_columns + column1_offset] > params.k_value) {
                row_array[data_count]  = dram[i * config_db.num_columns + column0_offset];
                data_count++;
            }
        }
        magic_timing_end(&cycleLo, &cycleHi);
        pmcs_get_value(&end);
        res = pmcs_diff(&end, &start);
        fprintf(params.output_file,"q2, d, -, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", config_db.row_size, config_db.row_count, params.col_width[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);

        // for (unsigned int i = 0; i < data_count; i++) {
        //     fprintf(params.results, "q2, %d, %d, %d, %d\n", config_db.row_size, cold_array[i], hot_array[i], row_array[i]);
        // }
    }


    if ( config_db.store_type == 'c' ){
      data_count = 0;
    	pmcs_get_value(&start);
    	magic_timing_begin(&cycleLo, &cycleHi);
      for (int i = 0; i < config_db.row_count; i++) {
          if (dram[config_db.row_count * column1_offset + i] > params.k_value) {
              col_array[data_count] = dram[config_db.row_count * column0_offset + i];
              data_count++;
          }
      }
    	magic_timing_end(&cycleLo, &cycleHi);
    	pmcs_get_value(&end);
    	res = pmcs_diff(&end, &start);
    	fprintf(params.output_file,"q2, c, -, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", config_db.row_size, config_db.row_count, params.col_width[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);
    }

    // Flushing the file stream
    fflush(params.output_file);
    fflush(params.results);

    munmap(plim, RELCACHE_SIZE);
    munmap(dram, dram_size);

    close(hpm_fd);
    close(dram_fd);

    free(cold_array);
    free(hot_array);
    free(row_array);
    free(col_array);

    int ret = teardown_pmcs();
    if (ret < 0)
        perror("Issue detected while tearing down the PMCs\n");
}

