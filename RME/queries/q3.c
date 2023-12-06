#define _GNU_SOURCE
#include "exp_header.h"
#include "performance_counters.h"

void run_query3(struct _config_db config_db, struct _config_query params){

    unsigned int cycleHi    = 0, cycleLo=0;
    struct perf_counters res, start, end;
    char store_type = 'r';
    T k = params.k_value;

    unsigned int column_num = config_db.row_size / sizeof(T);

    unsigned dram_size  = config_db.row_count*config_db.row_size;
    int hpm_fd          = open_fd();
    int dram_fd         = open_fd();

    int fd = setup_pmcs();
    if (fd < 0)
        perror("Issue opening PMC FDs\n");

    T *cold_array = malloc(config_db.row_count * sizeof(T));
    T *hot_array = malloc(config_db.row_count * sizeof(T));
    T *row_array = malloc(config_db.row_count * sizeof(T));
    
    
    unsigned char* plim = mmap(NULL, RELCACHE_SIZE, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_SHARED | 0x40, hpm_fd, RELCACHE_ADDR);
    unsigned char* dram = mmap(NULL, dram_size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_SHARED | 0x40, dram_fd, DRAM_ADDR);

    unsigned rme_row_size = 0;
    for (int i = 0; i < params.enabled_column_number; i++) {
        rme_row_size += config_db.column_widths[i];
    }

    T data;
    T data_count = 0;
    
    if ( config_db.store_type == 'r' ){

        T cold = 0;
        pmcs_get_value(&start);
        magic_timing_begin(&cycleLo, &cycleHi);
        for (int i = 0; i < config_db.row_count; i++) {
            T first_column_value = *(T*)(plim + i * rme_row_size + params.col_offsets[0]);
            T second_column_value = *(T*)(plim + i * rme_row_size + params.col_offsets[1]);
            if (first_column_value < k) {
                cold += second_column_value;
                cold_array[data_count] = second_column_value;
                data_count++;
            }
        }
        magic_timing_end(&cycleLo, &cycleHi);
        pmcs_get_value(&end);
        res = pmcs_diff(&end, &start);
        fprintf(params.output_file,"q3, r, c, %d, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", params.enabled_column_number, config_db.row_size, config_db.row_count, config_db.column_widths[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);
    
        T hot = 0;
        data_count = 0;
        pmcs_get_value(&start);
        magic_timing_begin(&cycleLo, &cycleHi);
        for (int i = 0; i < config_db.row_count; i++) {
            T first_column_value = *(T*)(plim + i * rme_row_size + params.col_offsets[0]);
            T second_column_value = *(T*)(plim + i * rme_row_size + params.col_offsets[1]);
            if (first_column_value < k) {
                hot += second_column_value;
                hot_array[data_count] = second_column_value;
                data_count++;
            }
        }
        magic_timing_end(&cycleLo, &cycleHi);
        pmcs_get_value(&end);
        res = pmcs_diff(&end, &start);
        fprintf(params.output_file,"q3, r, h, %d, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", params.enabled_column_number, config_db.row_size, config_db.row_count, config_db.column_widths[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);

        T row = 0;
        data_count = 0;
        pmcs_get_value(&start);
        magic_timing_begin(&cycleLo, &cycleHi);
        for (int i = 0; i < config_db.row_count; i++) {
            T first_column_value = *(T*)(dram + i * config_db.row_size + params.col_offsets[0]);
            T second_column_value = *(T*)(dram + i * config_db.row_size + params.col_offsets[1]);
            if (first_column_value < k) {
                row += second_column_value;
                row_array[data_count] = second_column_value;
                data_count++;
            }
        }
        magic_timing_end(&cycleLo, &cycleHi);
        pmcs_get_value(&end);
        res = pmcs_diff(&end, &start);
        fprintf(params.output_file,"q3, d, -, %d, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", params.enabled_column_number, config_db.row_size, config_db.row_count, config_db.column_widths[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);

        if (config_db.print == true){
          printf("\nQuery results:\n");
          printf("RME cold sum: %d\n", cold);
          printf("RME hot sum: %d\n", hot);
          printf("DRAM(row) sum: %d\n", row);
        }

        free(cold_array);
        free(hot_array);
        free(row_array);
    }

    if ( config_db.store_type == 'c' ){
      T col = 0;
      data_count = 0;
      T *col_array = malloc(config_db.row_count * sizeof(T));
    	pmcs_get_value(&start);
    	magic_timing_begin(&cycleLo, &cycleHi);
      for (int i = 0; i < config_db.row_count; i++) {
          T first_column_value = *(T*)(dram + config_db.row_count * params.col_offsets[0] + i * sizeof(T));
          T second_column_value = *(T*)(dram + config_db.row_count * params.col_offsets[1] + i * sizeof(T));
          if (first_column_value < k) {
              col += second_column_value;
              col_array[data_count] = second_column_value;
              data_count++;
          }
      }
    	magic_timing_end(&cycleLo, &cycleHi);
    	pmcs_get_value(&end);
    	res = pmcs_diff(&end, &start);
    	fprintf(params.output_file,"q3, c, -, %d, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", params.enabled_column_number, config_db.row_size, config_db.row_count, config_db.column_widths[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);
      free(col_array);   
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