#define _GNU_SOURCE
#include "query_header.h"

__uint128_t get_value( char * base, int width ) {
     __uint128_t data = 0;
     if ( width==1 ){
         unsigned char tmp = *(unsigned char*)(base);
         data = (__uint128_t) tmp;
     }
     else if ( width==2 ){
      unsigned short tmp = *(unsigned short *)(base);
         data = (__uint128_t) tmp;
     }
     else if ( width==4 ){
      unsigned int tmp = *(unsigned int *)(base);
         data = (__uint128_t) tmp;
     }
     else if ( width==8 ){
      unsigned long tmp = *(unsigned long *)(base);
         data = (__uint128_t) tmp;
     }
     return data;
}

int main(int argc, char** argv) {
    struct _config config;
    unsigned int cycleHi    = 0, cycleLo=0;
    char store_type = 'r';
    unsigned int k;

    unsigned char conditions[MAX_GROUPS];//22B
    unsigned char conditions_refined[MAX_GROUPS];//22B
    unsigned short conditions_offsets[MAX_GROUPS];//22B
    unsigned short projected_offsets[MAX_GROUPS];//22B
    unsigned short conditions_widths[MAX_GROUPS];//22B
    unsigned short projected_widths[MAX_GROUPS];//22B
    unsigned short conditions_abs_offsets[MAX_GROUPS];//22B
    unsigned short projected_abs_offsets[MAX_GROUPS];//22B
    for (int i=0 ; i<MAX_GROUPS ; i++){
        conditions[i] = '0';
        conditions_refined[i] = '0';
        conditions_abs_offsets[i] = 0;
        conditions_offsets[i] = 0;
        conditions_widths[i] = 0;
        projected_abs_offsets[i] = 0;
        projected_offsets[i] = 0;
        projected_widths[i] = 0;
    }
    unsigned int num_projected = 0;
    unsigned int num_conditions = 0;
    

    // -- pasring arguments -------------------------------------------
    int opt;
    while ((opt = getopt(argc, argv, "C:R:W:r:O:F:k:K:S:")) != -1) {
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
                k = (unsigned int)atoi(optarg); break;
            }
            case 'K': {
                char *pt;
                pt = strtok( optarg, "," );
                int i = 0;
                while (pt != NULL) {
                    if ( i >= 11 ){
                        fprintf(stderr, "Usage: [-K] The maximun number of columns is 11.\n");
                        exit(EXIT_FAILURE);
                    }
                    conditions[i] = *pt;
                    if ( conditions[i] != '0' && conditions[i] != 's' && conditions[i] != 'g' ){
                        fprintf(stderr, " Usage:   [-K] array of selection types (0: projection only, s:smaller, g:greater) (default: 0)\n");
                        exit(EXIT_FAILURE);
                    }
                    pt = strtok (NULL, ",");
                    i++;
                }
                break;
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
                fprintf(stderr, "          [-k] value for selections(int)\n");
                fprintf(stderr, "          [-K] array of selection types(\"cha, char, ..., char\") \n");
                exit(EXIT_FAILURE);
            }
        }
    }
    unsigned int total_width = 0;;
    unsigned int projection_width = 0;;
    for ( int i=0 ; i<config.enabled_col_num ; i++ ){
        if ( conditions[i] == '0' ){
            projected_abs_offsets[num_projected] = config.col_offsets[i];
            projected_offsets[num_projected] = total_width;
            projected_widths[num_projected] = config.col_widths[i];
            projection_width += config.col_widths[i];
            num_projected++;
        }
        else {
            conditions_abs_offsets[num_conditions] = config.col_offsets[i];
            conditions_offsets[num_conditions] = total_width;
            conditions_refined[num_conditions] = conditions[i];
            conditions_widths[num_conditions] = config.col_widths[i];
            num_conditions++;
        }
        total_width += config.col_widths[i];
    }
    if ( num_projected==0 ){
        fprintf(stderr, "There should be at least one projected column.\n");
        exit(EXIT_FAILURE);
    }
    // -- pasring arguments done --------------------------------------

    unsigned dram_size  = config.row_count*config.row_size;
#ifdef linux
    //mapping fpga:
    unsigned char * plim = (unsigned char *)malloc(RELCACHE_SIZE);
    //mapping dram
    unsigned char * dram = (unsigned char *)malloc(dram_size);
    for(int i = 0; i < config.row_count; i++){
        for(int j = 0; j < config.enabled_col_num; j++){
            *(unsigned int *)(plim + total_width*i + sizeof(unsigned int)*j ) = i;
        }
    }
    //for(int i = 0; i < config.row_count; i++){
    //    for(int j = 0; j < config.enabled_col_num; j++){
    //        printf( "%d\t", *(unsigned int *)(plim + total_width*i + sizeof(unsigned int)*j ) );
    //    }
    //    printf( "\n" );
    //}
#else
    int hpm_fd          = open_fd();
    int dram_fd         = open_fd();

    //mapping fpga:
    unsigned char * plim = mmap((void*)0, RELCACHE_SIZE, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED|0x40, hpm_fd, RELCACHE_ADDR);
    //mapping dram
    unsigned char * dram = mmap((void*)0, dram_size, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED|0x40, dram_fd, DRAM_ADDR);
#endif

    unsigned char * result = calloc( projection_width*config.row_count, sizeof(unsigned char) );

    //T data;
    __uint128_t data = 0;

    if ( store_type == 'r' ){
    magic_timing_begin(&cycleLo, &cycleHi);
    for(int i = 0; i < config.row_count; i++){
      for ( int j=0 ; j<num_conditions ; j++ ) {
    	data = get_value( plim + total_width*i + conditions_offsets[j], conditions_widths[j] );

        int multi = (conditions_refined[i]=='s')? -1 : 1;
        if ( (__int128)(multi*(data-k))<0 ) {
          continue;
        }
      }

      int tmp_width = 0;
      for ( int j=0 ; j<num_projected; j++ ) {
        tmp_width += projected_widths[j];
        memcpy( result+projection_width*i+tmp_width, plim+total_width*i+projected_offsets[j], projected_widths[j] );
      }
    }
    magic_timing_end(&cycleLo, &cycleHi);
    printf("q2, r, c, %d, %d, %d, %d\n", config.row_size, config.row_count, config.col_widths[0], cycleLo);

    magic_timing_begin(&cycleLo, &cycleHi);
    for(int i = 0; i < config.row_count; i++){
      for ( int j=0 ; j<num_conditions ; j++ ) {
    	data = get_value( plim + total_width*i + conditions_offsets[j], conditions_widths[j] );

        int multi = (conditions_refined[i]=='s')? -1 : 1;
        if ( (__int128)(multi*(data-k))<0 ) {
          continue;
        }
      }

      int tmp_width = 0;
      for ( int j=0 ; j<num_projected; j++ ) {
        tmp_width += projected_widths[j];
        memcpy( result+projection_width*i+tmp_width, plim+total_width*i+projected_offsets[j], projected_widths[j] );
      }
    }
    magic_timing_end(&cycleLo, &cycleHi);
    printf("q2, r, h, %d, %d, %d, %d\n", config.row_size, config.row_count, config.col_widths[0], cycleLo);

    magic_timing_begin(&cycleLo, &cycleHi);
    for(int i = 0; i < config.row_count; i++){
      for ( int j=0 ; j<num_conditions ; j++ ) {
    	data = get_value( dram + config.row_size*i + conditions_abs_offsets[j], conditions_widths[j] );

        int multi = (conditions_refined[i]=='s')? -1 : 1;
        if ( (__int128)(multi*(data-k))<0 ) {
          continue;
        }
      }

      int tmp_width = 0;
      for ( int j=0 ; j<num_projected; j++ ) {
        tmp_width += projected_widths[j];
        memcpy( result+projection_width*i+tmp_width, dram+config.row_size*i+projected_abs_offsets[j], projected_widths[j] );
      }
    }
    magic_timing_end(&cycleLo, &cycleHi);

    printf("q2, d, -, %d, %d, %d, %d\n", config.row_size, config.row_count, config.col_widths[0], cycleLo);
    }


    if ( store_type == 'c' ){
      magic_timing_begin(&cycleLo, &cycleHi);
      for(int i = 0; i < config.row_count; i++){
        for ( int j=0 ; j<num_conditions ; j++ ) {
      	data = get_value( dram + config.row_count*conditions_abs_offsets[j] + conditions_widths[j]*i, conditions_widths[j] );
  
          int multi = (conditions_refined[i]=='s')? -1 : 1;
          if ( (__int128)(multi*(data-k))<0 ) {
            continue;
          }
        }
  
        int tmp_width = 0;
        for ( int j=0 ; j<num_projected; j++ ) {
          tmp_width += projected_widths[j];
          memcpy( result+projection_width*i+tmp_width, dram+config.row_count*projected_abs_offsets[j] + conditions_widths[j]*i, projected_widths[j] );
        }
      }
      magic_timing_end(&cycleLo, &cycleHi);
  
      printf("q2, c, -, %d, %d, %d, %d\n", config.row_size, config.row_count, config.col_widths[0], cycleLo);
    }


    return 0;
}
