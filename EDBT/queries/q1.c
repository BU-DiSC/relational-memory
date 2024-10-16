#define _GNU_SOURCE
#include "exp_header.h"
#if IS_ARM
#include "performance_counters.h"
#endif

void run_query1(struct _config_db config_db, struct _config_query params) {

    unsigned int cycleHi    = 0, cycleLo=0;
    struct perf_counters res, start, end;
    int fd = setup_pmcs();
    if (fd < 0)
        perror("Issue opening PMC FDs\n");

    bool mvcc_enabled = false;
    T *cold_array = malloc(config_db.row_count * params.enabled_column_number * sizeof(T));
    T *hot_array = malloc(config_db.row_count * params.enabled_column_number * sizeof(T));
    T *row_array = malloc(config_db.row_count * params.enabled_column_number * sizeof(T));
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
    T data_count = 0;

	// Run RME 
	// b -> RME & ROW, r -> RME
    if ( config_db.store_type == 'b' || config_db.store_type == 'r'){
        // move multiplication outside
        unsigned width = config_db.column_widths[0];
        pmcs_get_value(&start);
        magic_timing_begin(&cycleLo, &cycleHi);
        for(int i = 0; i < config_db.row_count; i++){
          unsigned offset = 0;
          for(int j=0; j<params.enabled_column_number; j++){
            cold_array[data_count] = *(T*)(plim + i*sum_col_width + width*j);
            data_count++;
          }    
        }
        magic_timing_end(&cycleLo, &cycleHi);
        pmcs_get_value(&end);
        res = pmcs_diff(&end, &start);
        fprintf(params.output_file,"q1, r, c, %d, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", params.enabled_column_number, config_db.row_size, config_db.row_count, config_db.column_widths[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);

        data_count = 0;
        pmcs_get_value(&start);
        magic_timing_begin(&cycleLo, &cycleHi);
        for(int i = 0; i < config_db.row_count; i++){
          unsigned offset = 0;
          for(int j=0; j<params.enabled_column_number; j++){
            hot_array[data_count] = *(T*)(plim + i*sum_col_width + width*j);
            data_count++;
          }   
        }

        magic_timing_end(&cycleLo, &cycleHi);
        pmcs_get_value(&end);
        res = pmcs_diff(&end, &start);
        fprintf(params.output_file,"q1, r, h, %d, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", params.enabled_column_number, config_db.row_size, config_db.row_count, config_db.column_widths[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);
        
        data_count = 0;
        pmcs_get_value(&start);
        magic_timing_begin(&cycleLo, &cycleHi);
        for(int i = 0; i < config_db.row_count; i++){
          for(int j=0; j<params.enabled_column_number; j++){
               row_array[data_count] = *(T*)(dram + i*config_db.row_size + params.col_offsets[j]);
               data_count++;
          }
        }
        
        free(cold_array);
        free(hot_array);
        
    }
	
	//Run Row
	// b -> RME & ROW, d -> ROW
    if ( config_db.store_type == 'b' || config_db.store_type == 'd'){
		magic_timing_end(&cycleLo, &cycleHi);
        pmcs_get_value(&end);
        res = pmcs_diff(&end, &start);
        fprintf(params.output_file,"q1, d, -, %d, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", params.enabled_column_number, config_db.row_size, config_db.row_count, config_db.column_widths[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);
        if (config_db.print == true){
            printf("\nRow store query results:\n");
            printf("cold, hot, ROW\n");
            for (unsigned int i = 0; i < data_count; i++) {
                printf("%d, %d, %d\n", cold_array[i], hot_array[i], row_array[i]);
            }
        }
		free(row_array);
	}
	
	
	//Run Col
	// c -> COL
    if ( config_db.store_type == 'c' ){
		T *col_array = malloc(config_db.row_count * params.enabled_column_number * sizeof(T));
        data_count = 0;
        pmcs_get_value(&start);
        magic_timing_begin(&cycleLo, &cycleHi);
        for (int i = 0; i < config_db.row_count; i++) {
            for (int j = 0; j < params.enabled_column_number; j++) {
                // for now only for projectivity experiment
                col_array[data_count++] = *(T*)(dram + (i + j * config_db.row_count) * sizeof(T));
            }
        }
        magic_timing_end(&cycleLo, &cycleHi);
        pmcs_get_value(&end);
        res = pmcs_diff(&end, &start);
        fprintf(params.output_file,"q1, c, -, %d, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", params.enabled_column_number, config_db.row_size, config_db.row_count, config_db.column_widths[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);

        if (config_db.print == true){
            printf("\nColumn store query results:\n");
            for (unsigned int i = 0; i < data_count; i++) {
                printf("%d \n", col_array[i]);
            }
        }
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




























