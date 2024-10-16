#define _GNU_SOURCE
#include "exp_header.h"
// #include "performance_counters.h"
#if IS_ARM
#include "performance_counters.h"
#endif

void run_query2(struct _config_db config_db, struct _config_query params){    

    unsigned int cycleHi    = 0, cycleLo=0;
    struct perf_counters res, start, end;

    unsigned int column0_offset = params.col_offsets[0]/sizeof(T);
    unsigned int column1_offset = params.col_offsets[1]/sizeof(T);

    
    T data;

    T *cold_array = malloc(config_db.row_count * sizeof(T));
    T *hot_array = malloc(config_db.row_count * sizeof(T));
    T *row_array = malloc(config_db.row_count * sizeof(T));
    

    unsigned dram_size  = config_db.row_count*config_db.row_size;
    int hpm_fd          = open_fd();
    int dram_fd         = open_fd();

    int fd = setup_pmcs();
    if (fd < 0)
        perror("Issue opening PMC FDs\n");

    //mapping fpga:
    unsigned char* plim = mmap((void*)0, RELCACHE_SIZE, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED|0x40, hpm_fd, RELCACHE_ADDR);
    //mapping dram
    unsigned char* dram = mmap((void*)0, dram_size, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED|0x40, dram_fd, DRAM_ADDR);

    unsigned int data_count = 0;

    unsigned rme_row_size = 0;

    for ( int i=0 ; i<params.enabled_column_number ; i++ ) {
        rme_row_size += config_db.column_widths[i];
    }

	// Run RME
	// b -> RME & ROW, r -> RME
    if ( config_db.store_type == 'b' || config_db.store_type == 'r'){
        pmcs_get_value(&start);
        magic_timing_begin(&cycleLo, &cycleHi);
        for (int i = 0; i < config_db.row_count; i++) {
            T second_column_value = *(T*)(plim + i * rme_row_size + params.col_offsets[1]);
            if (second_column_value > params.k_value) {
                cold_array[data_count] = *(T*)(plim + i * rme_row_size + params.col_offsets[0]);
                data_count++;
            }
        }
        magic_timing_end(&cycleLo, &cycleHi);
        pmcs_get_value(&end);
        res = pmcs_diff(&end, &start);
        fprintf(params.output_file,"q2, r, c, %d, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", params.enabled_column_number, config_db.row_size, config_db.row_count, config_db.column_widths[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);
        
        data_count = 0;
        pmcs_get_value(&start);
        magic_timing_begin(&cycleLo, &cycleHi);
        for (int i = 0; i < config_db.row_count; i++) {
            T second_column_value = *(T*)(plim + i * rme_row_size + params.col_offsets[1]);
            if (second_column_value > params.k_value) {
                hot_array[data_count] = *(T*)(plim + i * rme_row_size + params.col_offsets[0]);
                data_count++;
            }
        }
        magic_timing_end(&cycleLo, &cycleHi);
        pmcs_get_value(&end);
        res = pmcs_diff(&end, &start);
        fprintf(params.output_file,"q2, r, h, %d, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", params.enabled_column_number, config_db.row_size, config_db.row_count, config_db.column_widths[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);
        

        if (config_db.print == true){
            printf("\nQuery results:\n");
            printf("cold, hot, ROW\n");
            for (unsigned int i = 0; i < data_count; i++) {
                printf("%d, %d, %d\n", cold_array[i], hot_array[i], row_array[i]);
            }
        }

        free(cold_array);
        free(hot_array);
        
    }

	// Run Row
	// b -> RME & ROW
	// d -> ROW
    if ( config_db.store_type == 'b' || config_db.store_type == 'd' ){
		data_count = 0;
        pmcs_get_value(&start);
        magic_timing_begin(&cycleLo, &cycleHi);
        for (int i = 0; i < config_db.row_count; i++) {
            T column1_value = *(T*)(dram + i * config_db.row_size + params.col_offsets[1]);

            if (column1_value > params.k_value) {
                row_array[data_count] = *(T*)(dram + i * config_db.row_size + params.col_offsets[0]);
                data_count++;
            }
        }
        magic_timing_end(&cycleLo, &cycleHi);
        pmcs_get_value(&end);
        res = pmcs_diff(&end, &start);
        fprintf(params.output_file,"q2, d, -, %d, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", params.enabled_column_number, config_db.row_size, config_db.row_count, config_db.column_widths[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);

		free(row_array);
    }

    // Run Col
	// c -> COL
	if ( config_db.store_type == 'c' ){
        data_count = 0;
        T *col_array = malloc(config_db.row_count * sizeof(T));
        // Compute the product of row_count and column offset outside the loop
    	pmcs_get_value(&start);
    	magic_timing_begin(&cycleLo, &cycleHi);

        unsigned int col0_offset_product = config_db.row_count * params.col_offsets[0];
        unsigned int col1_offset_product = config_db.row_count * params.col_offsets[1];
        for (int i = 0; i < config_db.row_count; i++) {
            T column1_value = *(T*)(dram + col1_offset_product + i * sizeof(T));
            if (column1_value > params.k_value) {
                col_array[data_count] = *(T*)(dram + col0_offset_product + i * sizeof(T));
                data_count++;
            }
        }
    	magic_timing_end(&cycleLo, &cycleHi);
    	pmcs_get_value(&end);
    	res = pmcs_diff(&end, &start);
    	fprintf(params.output_file,"q2, c, -, %d, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", params.enabled_column_number, config_db.row_size, config_db.row_count, config_db.column_widths[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);
        free(col_array);

    }

    fflush(params.output_file);

    munmap(plim, RELCACHE_SIZE);
    munmap(dram, dram_size);

    close(hpm_fd);
    close(dram_fd);

    int ret = teardown_pmcs();
    if (ret < 0)
        perror("Issue detected while tearing down the PMCs\n");
}