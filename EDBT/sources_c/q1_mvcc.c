#define _GNU_SOURCE
#include "query_header.h"

int main(int argc, char** argv) {
    struct _config config;
    unsigned int cycleHi    = 0, cycleLo=0;
    bool mvcc_enabled = false;
    char store_type = 'r';

    // -- pasring arguments -------------------------------------------
    int opt;
    while ((opt = getopt(argc, argv, "C:R:W:r:O:F:VS:")) != -1) {
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
            case 'V': {
                mvcc_enabled = true;
                break;
            }
            case 'S': {
                char type = *optarg;
                if ( type != 'r' && type != 'c' ){
                    fprintf(stderr, "Usage: %s [-S] r: row store, c: column store (default: r)\n", argv[0]);
                    exit(EXIT_FAILURE);
                }
                //printf("%s", optarg);
                store_type = type; break;
            }
            default: {
                fprintf(stderr, "Usage: %s [-C] number of columns (int) (default: 8)\n", argv[0]);
                fprintf(stderr, "          [-r] row size (int) (default: 64)\n");
                fprintf(stderr, "          [-R] number of row counts (int)\n");
                fprintf(stderr, "          [-W] array of column widths (\"int, int, ..., int\") \n");
                fprintf(stderr, "          [-O] array of column offsets (\"int, int, ..., int\") \n");
                fprintf(stderr, "          [-V] MVCC mode (bool)\n");
                fprintf(stderr, "          [-S] r: row store, c: column store (default: r)\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    unsigned mvcc_offset = 0;
    if ( mvcc_enabled ) {
        if ( config.enabled_col_num == 11 ){
            fprintf(stderr, "[-C] one column is reserved for MVCC timestamps. \n");
            exit(EXIT_FAILURE);
        }
        config.enabled_col_num++;
        config.col_offsets[config.enabled_col_num-1] = config.row_size;
        config.col_widths[config.enabled_col_num-1]  = 2*sizeof(time_t);
        mvcc_offset = config.row_size;
        config.row_size += 2*sizeof(time_t);
    }
    unsigned sum_col_width = 0;
    for ( int i=0 ; i<config.enabled_col_num ; i++ ) {
        sum_col_width += config.col_widths[i];
    }
    // -- pasring arguments done --------------------------------------

    unsigned dram_size  = config.row_count*config.row_size;
    int hpm_fd          = open_fd();
    int dram_fd         = open_fd();

    //mapping fpga:
    unsigned char* plim = mmap((void*)0, RELCACHE_SIZE, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED|0x40, hpm_fd, RELCACHE_ADDR);
    //mapping dram
    unsigned char* dram = mmap((void*)0, dram_size+config.frame_offset, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED|0x40, dram_fd, DRAM_ADDR);

    T data;

    if ( store_type == 'r' ){
        // move multiplication outside
        unsigned width = config.col_widths[0];
        magic_timing_begin(&cycleLo, &cycleHi);
        for(int i = 0; i < config.row_count; i++){
          // when the row is invalid
          if ( mvcc_enabled ){
            time_t start = *(unsigned long int*)(plim + i*sum_col_width + mvcc_offset );
            time_t end   = *(unsigned long int*)(plim + i*sum_col_width + mvcc_offset + sizeof(time_t) );
            if ( *(unsigned long int*)(plim + i*sum_col_width + mvcc_offset ) <= 0 ) {
                //printf("skip1 %lu\n", *(unsigned long int*)(plim + i*sum_col_width + mvcc_offset + (int)config.frame_offset));
                continue;
            }
            else if ( *(unsigned long int*)(plim + i*sum_col_width + mvcc_offset + sizeof(time_t)) != 0 
                   && *(unsigned long int*)(plim + i*sum_col_width + mvcc_offset + sizeof(time_t)) > *(unsigned long int*)(plim + i*sum_col_width + mvcc_offset) ) {
                //printf("skip2\n");
                continue;
            }
          }
          // when the row is valid
          unsigned offset = 0;
          for(int j=0; j<config.enabled_col_num - (int)mvcc_enabled; j++){
            data = *(T*)(plim + i*sum_col_width + width*j);
            //offset += config.col_widths[j];
          }    
        }

        magic_timing_end(&cycleLo, &cycleHi);
        printf("q1, r, c, %d, %d, %d, %d, %d\n", mvcc_enabled, config.row_size, config.row_count, config.enabled_col_num, cycleLo);

        magic_timing_begin(&cycleLo, &cycleHi);
        for(int i = 0; i < config.row_count; i++){
          // when the row is invalid
          if ( mvcc_enabled ){
            if ( *(unsigned long int*)(plim + i*sum_col_width + mvcc_offset ) <= 0 ) {
                continue;
            }
            else if ( *(unsigned long int*)(plim + i*sum_col_width + mvcc_offset + sizeof(time_t)) != 0 
                   && *(unsigned long int*)(plim + i*sum_col_width + mvcc_offset + sizeof(time_t)) > *(unsigned long int*)(plim + i*sum_col_width + mvcc_offset) ) {
                continue;
            }
          }
          // when the row is valid
          unsigned offset = 0;
          for(int j=0; j<config.enabled_col_num - (int)mvcc_enabled; j++){
            data = *(T*)(plim + i*sum_col_width + offset);
            offset += config.col_widths[j];
          }
        }

        magic_timing_end(&cycleLo, &cycleHi);
        printf("q1, r, h, %d, %d, %d, %d, %d\n", mvcc_enabled, config.row_size, config.row_count, config.enabled_col_num, cycleLo);

        magic_timing_begin(&cycleLo, &cycleHi);
        for(int i = 0; i < config.row_count; i++){
          // when the row is invalid
          if ( mvcc_enabled ){
            if ( *(unsigned long int*)(dram + i*config.row_size + config.col_offsets[config.enabled_col_num-1] + (int)config.frame_offset) <= 0 ) {
                continue;
            }
            else if ( *(unsigned long int*)(dram + i*config.row_size + config.col_offsets[config.enabled_col_num-1] + sizeof(time_t) + (int)config.frame_offset) != 0 && (time_t) dram[(i*config.row_size + config.col_offsets[config.enabled_col_num-1] + sizeof(time_t)) + (int)config.frame_offset] > *(unsigned long int*)(dram + i*sum_col_width + mvcc_offset + (int)config.frame_offset) ) {
                continue;
            }
          }
          // when the row is valid
          for(int j=0; j<config.enabled_col_num - (int)mvcc_enabled; j++){
              data = *(T*)(dram + i*config.row_size + config.col_offsets[j] + (int)config.frame_offset);
          }
        }
        magic_timing_end(&cycleLo, &cycleHi);

        printf("q1, d, -, %d, %d, %d, %d, %d\n", mvcc_enabled, config.row_size, config.row_count, config.enabled_col_num, cycleLo);
    }
    else if ( store_type == 'c' ){

        // columnar access
        unsigned char* result = (unsigned char *)malloc(config.enabled_col_num * sizeof(T));

#ifdef __FOR
        magic_timing_begin(&cycleLo, &cycleHi);
        for(int i = 0; i < config.row_count; i++)
        {
          // when the row is invalid
          if ( mvcc_enabled ){
            if ( *(unsigned long int*)(dram + i*sizeof(time_t) + config.row_count * config.col_offsets[config.enabled_col_num-1] + (int)config.frame_offset) <= 0 ) {
                continue;
            }
            else if ( *(unsigned long int*)(dram + i*sizeof(time_t) + config.row_count*(config.col_offsets[config.enabled_col_num-1] + sizeof(time_t)) + (int)config.frame_offset) != 0 
                   && *(unsigned long int*)(dram + i*sizeof(time_t) + config.row_count*(config.col_offsets[config.enabled_col_num-1] + sizeof(time_t)) + (int)config.frame_offset) > *(unsigned long int*)(dram + i*sizeof(time_t) + config.row_count * config.col_offsets[config.enabled_col_num-1] + (int)config.frame_offset) ) {
                continue;
            }
          }
          // when the row is valid
          for(int j=0; j<config.enabled_col_num - (int)mvcc_enabled; j++){
              //memcpy(result + j, (dram + i*config.col_widths[j] + config.row_count*config.col_offsets[j] + (int)config.frame_offset), sizeof(T));
              *(result+j) = *(T*)(dram + i*config.col_widths[j] + config.row_count*config.col_offsets[j] + (int)config.frame_offset);
          }
        }
        magic_timing_end(&cycleLo, &cycleHi);

        printf("q1, c, -, %d, %d, %d, %d, %d\n", mvcc_enabled, config.row_size, config.row_count, config.enabled_col_num, cycleLo);

#else
        magic_timing_begin(&cycleLo, &cycleHi);
        for(int i = 0; i < config.row_count; i++)
        {
          // when the row is invalid
          if ( mvcc_enabled ){
            if ( *(unsigned long int*)(dram + i*sizeof(time_t) + config.row_count * config.col_offsets[config.enabled_col_num-1] + (int)config.frame_offset) <= 0 ) {
                continue;
            }
            else if ( *(unsigned long int*)(dram + i*sizeof(time_t) + config.row_count*(config.col_offsets[config.enabled_col_num-1] + sizeof(time_t)) + (int)config.frame_offset) != 0 
                   && *(unsigned long int*)(dram + i*sizeof(time_t) + config.row_count*(config.col_offsets[config.enabled_col_num-1] + sizeof(time_t)) + (int)config.frame_offset) > *(unsigned long int*)(dram + i*sizeof(time_t) + config.row_count * config.col_offsets[config.enabled_col_num-1] + (int)config.frame_offset) ) {
                continue;
            }
          }
          // when the row is valid
          if (config.enabled_col_num-mvcc_enabled == 1){
            memcpy(result, (dram + i), sizeof(T));
          }
          else if (config.enabled_col_num-mvcc_enabled == 2){
            memcpy(result, (dram + i), sizeof(T));
            memcpy(result + 1, (dram + i + config.row_count), sizeof(T));
          }
          else if (config.enabled_col_num-mvcc_enabled == 3){
            memcpy(result, (dram + i), sizeof(T));
            memcpy(result + 1, (dram + i + config.row_count), sizeof(T));
            memcpy(result + 2, (dram + i + 2*config.row_count), sizeof(T));
          }
          else if (config.enabled_col_num-mvcc_enabled == 4){
            memcpy(result, (dram + i), sizeof(T));
            memcpy(result + 1, (dram + i + config.row_count), sizeof(T));
            memcpy(result + 2, (dram + i + 2*config.row_count), sizeof(T));
            memcpy(result + 3, (dram + i + 3*config.row_count), sizeof(T));
          }
          else if (config.enabled_col_num-mvcc_enabled == 5){
            memcpy(result, (dram + i), sizeof(T));
            memcpy(result + 1, (dram + i + config.row_count), sizeof(T));
            memcpy(result + 2, (dram + i + 2*config.row_count), sizeof(T));
            memcpy(result + 3, (dram + i + 3*config.row_count), sizeof(T));
            memcpy(result + 4, (dram + i + 4*config.row_count), sizeof(T));
          }
          else if (config.enabled_col_num-mvcc_enabled == 6){
            memcpy(result, (dram + i), sizeof(T));
            memcpy(result + 1, (dram + i + config.row_count), sizeof(T));
            memcpy(result + 2, (dram + i + 2*config.row_count), sizeof(T));
            memcpy(result + 3, (dram + i + 3*config.row_count), sizeof(T));
            memcpy(result + 4, (dram + i + 4*config.row_count), sizeof(T));
            memcpy(result + 5, (dram + i + 5*config.row_count), sizeof(T));
          }
          else if (config.enabled_col_num-mvcc_enabled == 7){
            memcpy(result, (dram + i), sizeof(T));
            memcpy(result + 1, (dram + i + config.row_count), sizeof(T));
            memcpy(result + 2, (dram + i + 2*config.row_count), sizeof(T));
            memcpy(result + 3, (dram + i + 3*config.row_count), sizeof(T));
            memcpy(result + 4, (dram + i + 4*config.row_count), sizeof(T));
            memcpy(result + 5, (dram + i + 5*config.row_count), sizeof(T));
            memcpy(result + 6, (dram + i + 6*config.row_count), sizeof(T));
          }
          else if (config.enabled_col_num-mvcc_enabled == 8){
            memcpy(result, (dram + i), sizeof(T));
            memcpy(result + 1, (dram + i + config.row_count), sizeof(T));
            memcpy(result + 2, (dram + i + 2*config.row_count), sizeof(T));
            memcpy(result + 3, (dram + i + 3*config.row_count), sizeof(T));
            memcpy(result + 4, (dram + i + 4*config.row_count), sizeof(T));
            memcpy(result + 5, (dram + i + 5*config.row_count), sizeof(T));
            memcpy(result + 6, (dram + i + 6*config.row_count), sizeof(T));
            memcpy(result + 7, (dram + i + 7*config.row_count), sizeof(T));
          }
          else if (config.enabled_col_num-mvcc_enabled == 9){
            memcpy(result, (dram + i), sizeof(T));
            memcpy(result + 1, (dram + i + config.row_count), sizeof(T));
            memcpy(result + 2, (dram + i + 2*config.row_count), sizeof(T));
            memcpy(result + 3, (dram + i + 3*config.row_count), sizeof(T));
            memcpy(result + 4, (dram + i + 4*config.row_count), sizeof(T));
            memcpy(result + 5, (dram + i + 5*config.row_count), sizeof(T));
            memcpy(result + 6, (dram + i + 6*config.row_count), sizeof(T));
            memcpy(result + 7, (dram + i + 7*config.row_count), sizeof(T));
            memcpy(result + 8, (dram + i + 8*config.row_count), sizeof(T));
          }
          else if (config.enabled_col_num-mvcc_enabled == 10){
            memcpy(result, (dram + i), sizeof(T));
            memcpy(result + 1, (dram + i + config.row_count), sizeof(T));
            memcpy(result + 2, (dram + i + 2*config.row_count), sizeof(T));
            memcpy(result + 3, (dram + i + 3*config.row_count), sizeof(T));
            memcpy(result + 4, (dram + i + 4*config.row_count), sizeof(T));
            memcpy(result + 5, (dram + i + 5*config.row_count), sizeof(T));
            memcpy(result + 6, (dram + i + 6*config.row_count), sizeof(T));
            memcpy(result + 7, (dram + i + 7*config.row_count), sizeof(T));
            memcpy(result + 8, (dram + i + 8*config.row_count), sizeof(T));
            memcpy(result + 9, (dram + i + 9*config.row_count), sizeof(T));
          }
          
        }
        magic_timing_end(&cycleLo, &cycleHi);
        printf("q1, c, -, %d, %d, %d, %d, %d\n", mvcc_enabled, config.row_size, config.row_count, config.enabled_col_num, cycleLo);
#endif
    }
    
    return 0;
}
