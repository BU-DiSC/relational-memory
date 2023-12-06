#define _GNU_SOURCE
#include "exp_header.h"
#include "performance_counters.h"

int already_grouped(T check, int group_array_counter, T* group_array){
  int grouped = 0;
  for(int i=0; i<group_array_counter; i++){
    if(check == group_array[i]){
      grouped = 1;
      //printf("Data %llx is already grouped\n", check);
      return grouped;
    }
  }
  //printf("Data %llx is NOT grouped\n", check);
  return grouped;
}

void run_query4(struct _config_db config_db, struct _config_query params){
    
    unsigned int cycleHi    = 0, cycleLo=0;
    struct perf_counters res, start, end;
    char store_type = 'r';
    T k = params.k_value;

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

    if ( config_db.store_type == 'r' ){
    T* plim_group_array = NULL;
    int plim_group_array_capacity = 16;
    plim_group_array = (T*)malloc(plim_group_array_capacity * sizeof(T));
    if (!plim_group_array) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }
    int plim_group_array_counter = 0;
    T plim_average = 0;
    int plim_repetition = 0;
    
    pmcs_get_value(&start);
    magic_timing_begin(&cycleLo, &cycleHi);
    for(int i = 0; i < config_db.row_count; i++){
      if (plim[i*params.enabled_column_number + 2] > k) {
        if(!already_grouped((plim[i*params.enabled_column_number+1]&((T)0xFF)), plim_group_array_counter, plim_group_array)) {
            // Check if a resize is needed before insertion
            if (plim_group_array_counter >= plim_group_array_capacity) {
                plim_group_array_capacity *= 2;  // double the capacity
                plim_group_array = (T*)realloc(plim_group_array, plim_group_array_capacity * sizeof(T));
                if (!plim_group_array) {
                    fprintf(stderr, "Memory reallocation failed!\n");
                    exit(EXIT_FAILURE);
                }
            }
            
            plim_group_array[plim_group_array_counter++] = (plim[i*params.enabled_column_number+1]&((T)0xFF));
            plim_repetition = 1;
            plim_average = plim[i*params.enabled_column_number];
            
            for(int j = i+1; j < config_db.row_count; j++){
              if(plim[j*params.enabled_column_number+2] > k){
                if(plim[j*params.enabled_column_number+1] == plim[i*params.enabled_column_number+1]){
                  plim_repetition++;
                  plim_average += plim[j*params.enabled_column_number];
                }
              }
            }
            plim_average /= plim_repetition;
            //printf("RME cold Group: Average: %u\n", plim_average);
        }
      }
    }
    magic_timing_end(&cycleLo, &cycleHi);
    pmcs_get_value(&end);
    res = pmcs_diff(&end, &start);
    fprintf(params.output_file,"q4, r, c, %d, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", params.enabled_column_number, config_db.row_size, config_db.row_count, config_db.column_widths[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);
    free(plim_group_array);

    plim_group_array = NULL;
    plim_group_array_capacity = 16;
    plim_group_array = (T*)malloc(plim_group_array_capacity * sizeof(T));
    if (!plim_group_array) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }
    plim_group_array_counter = 0;
    plim_average = 0;
    plim_repetition = 0;
    
    // start RME hot
    pmcs_get_value(&start);
    magic_timing_begin(&cycleLo, &cycleHi);
    for(int i = 0; i < config_db.row_count; i++){
      if (plim[i*params.enabled_column_number + 2] > k) {
        if(!already_grouped((plim[i*params.enabled_column_number+1]&((T)0xFF)), plim_group_array_counter, plim_group_array)) {
            // Check if a resize is needed before insertion
            if (plim_group_array_counter >= plim_group_array_capacity) {
                plim_group_array_capacity *= 2;  // double the capacity
                plim_group_array = (T*)realloc(plim_group_array, plim_group_array_capacity * sizeof(T));
                if (!plim_group_array) {
                    fprintf(stderr, "Memory reallocation failed!\n");
                    exit(EXIT_FAILURE);
                }
            }
            
            plim_group_array[plim_group_array_counter++] = (plim[i*params.enabled_column_number+1]&((T)0xFF));
            plim_repetition = 1;
            plim_average = plim[i*params.enabled_column_number];
            
            for(int j = i+1; j < config_db.row_count; j++){
              if(plim[j*params.enabled_column_number+2] > k){
                if(plim[j*params.enabled_column_number+1] == plim[i*params.enabled_column_number+1]){
                  plim_repetition++;
                  plim_average += plim[j*params.enabled_column_number];
                }
              }
            }
            plim_average /= plim_repetition;
            //printf("rme hot Group Average: %u\n", (plim[i*params.enabled_column_number+1]&((T)0xFF)), plim_average);
        }
      }
    }
    magic_timing_end(&cycleLo, &cycleHi);
    pmcs_get_value(&end);
    res = pmcs_diff(&end, &start);
    fprintf(params.output_file,"q4, r, h, %d, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", params.enabled_column_number, config_db.row_size, config_db.row_count, config_db.column_widths[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);
    free(plim_group_array);

    // start DRAM
    T* dram_group_array = NULL;
    int dram_group_array_capacity = 16;
    dram_group_array = (T*)malloc(dram_group_array_capacity * sizeof(T));
    if (!dram_group_array) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }
    int dram_group_array_counter = 0;
    plim_average = 0;  // Keeping the naming as plim_average for consistency
    plim_repetition = 0;

    pmcs_get_value(&start);
    magic_timing_begin(&cycleLo, &cycleHi);

    for(int i = 0; i < config_db.row_count; i++) {
        if (dram[(i*config_db.row_size + params.col_offsets[2])/sizeof(T)] > k) {
            if(!already_grouped((dram[(i*config_db.row_size + params.col_offsets[1])/sizeof(T)]&((T)0xFF)), dram_group_array_counter, dram_group_array)) {
                // Check if a resize is needed before insertion
                if (dram_group_array_counter >= dram_group_array_capacity) {
                    dram_group_array_capacity *= 2;  // double the capacity
                    dram_group_array = (T*)realloc(dram_group_array, dram_group_array_capacity * sizeof(T));
                    if (!dram_group_array) {
                        fprintf(stderr, "Memory reallocation failed!\n");
                        exit(EXIT_FAILURE);
                    }
                }

                dram_group_array[dram_group_array_counter++] = (dram[(i*config_db.row_size + params.col_offsets[1])/sizeof(T)]&((T)0xFF));
                plim_repetition = 1;
                plim_average = dram[(i*config_db.row_size)/sizeof(T)];

                for(int j = i+1; j < config_db.row_count; j++) {
                    if(dram[(j*config_db.row_size + params.col_offsets[2])/sizeof(T)] > k) {
                        if(dram[(j*config_db.row_size + params.col_offsets[1])/sizeof(T)] == dram[(i*config_db.row_size + params.col_offsets[1])/sizeof(T)]) {
                            plim_repetition++;
                            plim_average += dram[(j*config_db.row_size)/sizeof(T)];
                        }
                    }
                }
                plim_average /= plim_repetition;
                //printf("row Group:  Average: %u\n",  plim_average);
            }
        }
    }

    magic_timing_end(&cycleLo, &cycleHi);
    pmcs_get_value(&end);
    res = pmcs_diff(&end, &start);
    fprintf(params.output_file,"q4, d, -, %d, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", params.enabled_column_number, config_db.row_size, config_db.row_count, config_db.column_widths[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);

    free(dram_group_array);  // Free the dynamically allocated memory
    }
    //************************************* start dram column store *******************************************
    
    if ( config_db.store_type == 'c' ){
    T* dram_group_array = NULL;
    int dram_group_array_capacity = 16;
    dram_group_array = (T*)malloc(dram_group_array_capacity * sizeof(T));
    if (!dram_group_array) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }
    int dram_group_array_counter = 0;
    T plim_average = 0;
    int plim_repetition = 0;

    pmcs_get_value(&start);
    magic_timing_begin(&cycleLo, &cycleHi);

    for(int i = 0; i < config_db.row_count; i++) {
        if (dram[(i + config_db.row_count * (params.col_offsets[2])/sizeof(T))] > k) {
            if(!already_grouped((dram[(i + config_db.row_count * (params.col_offsets[1])/sizeof(T))]&((T)0xFF)), dram_group_array_counter, dram_group_array)) {
                // Check if a resize is needed before insertion
                if (dram_group_array_counter >= dram_group_array_capacity) {
                    dram_group_array_capacity *= 2;  // double the capacity
                    dram_group_array = (T*)realloc(dram_group_array, dram_group_array_capacity * sizeof(T));
                    if (!dram_group_array) {
                        fprintf(stderr, "Memory reallocation failed!\n");
                        exit(EXIT_FAILURE);
                    }
                }

                dram_group_array[dram_group_array_counter++] = (dram[(i + config_db.row_count * (params.col_offsets[1])/sizeof(T))]&((T)0xFF));
                plim_repetition = 1;
                plim_average = dram[(i + config_db.row_count * (params.col_offsets[0])/sizeof(T))];

                for(int j = i+1; j < config_db.row_count; j++) {
                    if(dram[(j + config_db.row_count * (params.col_offsets[2])/sizeof(T))] > k) {
                        if(dram[j + config_db.row_count * (params.col_offsets[1]/sizeof(T))] == dram[i + config_db.row_count * (params.col_offsets[1])/sizeof(T)]) {
                            plim_repetition++;
                            plim_average += dram[(j*config_db.row_size)/sizeof(T)];
                        }
                    }
                }
                plim_average /= plim_repetition;
                //printf("column Group: Average: %u\n", plim_average);
            }
        }
    }

    magic_timing_end(&cycleLo, &cycleHi);
    pmcs_get_value(&end);
    res = pmcs_diff(&end, &start);
    fprintf(params.output_file,"q4, c, -, %d, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu\n", params.enabled_column_number, config_db.row_size, config_db.row_count, config_db.column_widths[0], cycleLo, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired);

    free(dram_group_array);  // Free the dynamically allocated memory
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