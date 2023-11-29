#define _GNU_SOURCE
#include "config.h" 
#include "print_utils.h"

int configure_relcache(struct _config_db config_db, struct _config_query *params) {

    unsigned int   frame_offset = 0;
    int lpd_fd  = open_fd();
    struct _config* config = (struct _config *)mmap(NULL, LPD0_SIZE, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED, lpd_fd, LPD0_ADDR);

    config->row_size = config_db.row_size;
    config->row_count = config_db.row_count;
    config->enabled_col_num = params->enabled_column_number;
    
    for(int i=0; i<params->enabled_column_number; i+=1){
      config->col_widths[i] = params->col_width[i];
    }
    unsigned short sum_col_offsets = 0;
    for(int i=0; i<params->enabled_column_number; i+=1){
      config->col_offsets[i] = params->col_offsets[i] - sum_col_offsets;
      sum_col_offsets = params->col_offsets[i];   
    }

    config->frame_offset = frame_offset;

    //print_config_info(config);

    int unmap_result = munmap(config, LPD0_SIZE);
    return unmap_result;
}

#define __dsb(){\
  do{\
    asm volatile("dsb 15");\
  }while(0);\
}

int reset_relcache(unsigned int frame_offset) {  
    
    int lpd_fd  = open_fd();
    struct _config* config = (struct _config *)mmap((void*)0, LPD0_SIZE, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED, lpd_fd, LPD0_ADDR);
    __dsb();
    // reset
    config->frame_offset = frame_offset; 
    unsigned int reset_data = config->reset;
    config->reset = (reset_data + 1) & 0x1; 
    __dsb();
    //unmap
    int unmap_result = munmap(config, LPD0_SIZE);
    return unmap_result;
}

void call_db_reset_relcache(int value) {
    const char* ODIR = "objects";
    char cmd[256];

    // Check if the provided value is 0 or 1
    if (value != 0 && value != 1) {
        fprintf(stderr, "Invalid value. Only 0 or 1 accepted.\n");
        return;
    }

    // Prepare the command string
    snprintf(cmd, sizeof(cmd), "%s/db_reset_relcache %d", ODIR, value);

    // Execute the command
    system(cmd);
}

