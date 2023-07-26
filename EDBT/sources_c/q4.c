#define _GNU_SOURCE
#include "query_header.h"

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

    T plim_group_array[MAX_GROUPS];
    int plim_group_array_counter   = 0;
    T plim_average = 0;
    int plim_repetition = 0;

    // assuming 1st column used for selection
    magic_timing_begin(&cycleLo, &cycleHi);
    for(int i = 0; i < config.row_count; i++){
      if (plim[i*config.enabled_col_num + 2] != k)
      {
        if( !already_grouped((plim[i*config.enabled_col_num+1]&((T)0xFF)), plim_group_array_counter, plim_group_array)){
          plim_group_array[plim_group_array_counter++] = (plim[i*config.enabled_col_num+1]&((T)0xFF));
          plim_repetition = 1;
          plim_average = plim[i*config.enabled_col_num];
          for(int j = i+1; j < config.row_count; j++){
            if(plim[j*config.enabled_col_num+2] != k){
              if(plim[j*config.enabled_col_num+1] == plim[i*config.enabled_col_num+1]){
              plim_repetition++;
              plim_average += plim[j*config.enabled_col_num];
              }
            }
          }
          plim_average /= plim_repetition;
        }        
      }    
    }
    magic_timing_end(&cycleLo, &cycleHi);
    printf("q4, r, c, %d, %d, %d, %d\n", config.row_size, config.row_count, config.col_widths[0], cycleLo);


    T hot_plim_group_array[MAX_GROUPS];
    plim_group_array_counter   = 0;
    plim_average = 0;
    plim_repetition = 0;
    magic_timing_begin(&cycleLo, &cycleHi);
    for(int i = 0; i < config.row_count; i++){
      if (plim[i*config.enabled_col_num + 2] != k)
      {
        if( !already_grouped((plim[i*config.enabled_col_num+1]&((T)0xFF)), plim_group_array_counter, hot_plim_group_array)){
          hot_plim_group_array[plim_group_array_counter++] = (plim[i*config.enabled_col_num+1]&((T)0xFF));
          plim_repetition = 1;
          plim_average = plim[i*config.enabled_col_num];
          for(int j = i+1; j < config.row_count; j++){
            if(plim[j*config.enabled_col_num+2] != k){
              if(plim[j*config.enabled_col_num+1] == plim[i*config.enabled_col_num+1]){
              plim_repetition++;
              plim_average += plim[j*config.enabled_col_num];
              }
            }
          }
          plim_average /= plim_repetition;
        }        
      }    
    }
    magic_timing_end(&cycleLo, &cycleHi);
    printf("q4, r, h, %d, %d, %d, %d\n", config.row_size, config.row_count, config.col_widths[0], cycleLo);

    T dram_group_array[MAX_GROUPS];
    plim_group_array_counter   = 0;
    plim_average = 0;
    plim_repetition = 0;
    magic_timing_begin(&cycleLo, &cycleHi);
    for(int i = 0; i < config.row_count; i++){
        if (dram[(i*config.row_size + config.col_offsets[2])/sizeof(T)] != k)
        {
            if( !already_grouped((dram[(i*config.row_size + config.col_offsets[1])/sizeof(T)]&((T)0xFF)), plim_group_array_counter, dram_group_array)){
              dram_group_array[plim_group_array_counter++] = (dram[(i*config.row_size + config.col_offsets[1])/sizeof(T)]&((T)0xFF));
              plim_repetition = 1;
              plim_average = dram[(i*config.row_size)/sizeof(T)];
              for(int j = i+1; j < config.row_count; j++){
                if(dram[(j*config.row_size + config.col_offsets[2])/sizeof(T)] != k){
                  if(dram[(j*config.row_size + config.col_offsets[1])/sizeof(T)] == dram[(i*config.row_size + config.col_offsets[1])/sizeof(T)]){
                  plim_repetition++;
                  plim_average += dram[(j*config.row_size)/sizeof(T)];
                  }
                }
              }
              plim_average /= plim_repetition;
            }        
        }
    }
    magic_timing_end(&cycleLo, &cycleHi);

    printf("q4, d, -, %d, %d, %d, %d\n", config.row_size, config.row_count, config.col_widths[0], cycleLo);

    return 0;
}
