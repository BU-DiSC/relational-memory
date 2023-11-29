#define _GNU_SOURCE
#include "exp_header.h"
#include "performance_counters.h"

void run_query3(struct _config_db config_db, struct _config_query params){

    unsigned int cycleHi    = 0, cycleLo=0;
    struct perf_counters res, start, end;
    char store_type = 'r';
    T k = params.k_value;

    unsigned int column_num = config_db.row_size / sizeof(T);
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
    T* dram = mmap((void*)0, dram_size, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED|0x40, dram_fd, DRAM_ADDR);

    T *cold_array = malloc(config_db.row_count * sizeof(T));
    T *hot_array = malloc(config_db.row_count * sizeof(T));
    T *row_array = malloc(config_db.row_count * sizeof(T));
    T *col_array = malloc(config_db.row_count * sizeof(T));

    T data;
    T data_count = 0;
    T sumq = 0;
    if ( config_db.store_type == 'r' ){
        // assuming 2nd column used for selection
        pmcs_get_value(&start);
        magic_timing_begin(&cycleLo, &cycleHi);
        for(int i = 0; i < config_db.row_count; i++){
          if (plim[i*params.enabled_column_number] < k)
          {
             data = plim[i*params.enabled_column_number + 1];
             cold_array[data_count++] = data;
             sumq += data;
          }    
        }
        magic_timing_end(&cycleLo, &cycleHi);
        pmcs_get_value(&end);
        res = pmcs_diff(&end, &start);
        fprintf(params.output_file,"q3, r, c, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", config_db.row_size, config_db.row_count, params.col_width[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);
        printf("RME hot sum %u\n",sumq);
    
        sumq = 0;
        data_count = 0;
        pmcs_get_value(&start);
        magic_timing_begin(&cycleLo, &cycleHi);
        for(int i = 0; i < config_db.row_count; i++){
          if (plim[i*params.enabled_column_number] < k)
          {
             data = plim[i*params.enabled_column_number + 1];
             hot_array[data_count++] = data;
             sumq += data;
          }    
        }
        magic_timing_end(&cycleLo, &cycleHi);
        pmcs_get_value(&end);
        res = pmcs_diff(&end, &start);
        fprintf(params.output_file,"q3, r, h, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", config_db.row_size, config_db.row_count, params.col_width[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);
        printf("RME cold sum %u\n",sumq);

        sumq = 0;
        data_count = 0;
        pmcs_get_value(&start);
        magic_timing_begin(&cycleLo, &cycleHi);
        for(int i = 0; i < config_db.row_count; i++){
          if (dram[i * column_num + column0_offset] < k)
          {
            data = dram[i * column_num + column1_offset];
            row_array[data_count++] = data;
            sumq += data;
          }
        }
        magic_timing_end(&cycleLo, &cycleHi);
        pmcs_get_value(&end);
        res = pmcs_diff(&end, &start);
        fprintf(params.output_file,"q3, d, -, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", config_db.row_size, config_db.row_count, params.col_width[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);
        printf("DRAM sum %u\n",sumq);
    }
    sumq = 0;
    data_count = 0;
    if ( config_db.store_type == 'c' ){
    	pmcs_get_value(&start);
    	magic_timing_begin(&cycleLo, &cycleHi);
    	for(int i = 0; i < config_db.row_count; i++)
    	{
    	  if (dram[(i + config_db.row_count * column0_offset)] < k)
    	  {
    	    data = dram[i + config_db.row_count * column1_offset];
          col_array[data_count++] = data;
          sumq += data;
    	  }
    	}
    	magic_timing_end(&cycleLo, &cycleHi);
    	pmcs_get_value(&end);
    	res = pmcs_diff(&end, &start);
    	fprintf(params.output_file,"q3, c, -, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", config_db.row_size, config_db.row_count, params.col_width[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);
      printf("DRAM column store: %d\n", sumq);
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