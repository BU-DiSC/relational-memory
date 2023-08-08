#define _GNU_SOURCE
#include "query_header.h"

int main(int argc, char** argv) {
    unsigned int row_size;
    unsigned int row_count;
    unsigned int enabled_col_num;
    unsigned short col_widths[MAX_GROUPS];//We support maximum of 11 columns
    unsigned short col_offsets[MAX_GROUPS];
    unsigned int   frame_offset = 0;
    bool mvcc_enabled = false;

    // -- pasring arguments -------------------------------------------
    int opt;
    while ((opt = getopt(argc, argv, "C:R:W:r:O:F:V")) != -1) {
        switch (opt) {
            case 'C': {
                enabled_col_num = atoi(optarg); break;
            }
            case 'R': {
                row_count = atoi(optarg); break;
            }
            case 'r': {
                row_size = atoi(optarg); break;
            }
            case 'F': {
                frame_offset = atoi(optarg); break;
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
                    col_widths[i] = atoi(pt);
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
                    col_offsets[i] = atoi(pt);
                    if ( i > 0 ){
                        if ( col_offsets[i] <= col_offsets[i-1]){
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
            default: {
                fprintf(stderr, "Usage: %s [-C] number of columns (int) (default: 8)\n", argv[0]);
                fprintf(stderr, "          [-r] row size (int) (default: 64)\n");
                fprintf(stderr, "          [-R] number of row counts (int)\n");
                fprintf(stderr, "          [-W] array of column widths (\"int, int, ..., int\") \n");
                fprintf(stderr, "          [-O] array of column offsets (\"int, int, ..., int\") \n");
                fprintf(stderr, "          [-V] MVCC mode (bool)\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    unsigned mvcc_offset = 0;
    if ( mvcc_enabled ) {
        if ( enabled_col_num == 11 ){
            fprintf(stderr, "[-C] one column is reserved for MVCC timestamps. \n");
            exit(EXIT_FAILURE);
        }
        enabled_col_num++;
        col_offsets[enabled_col_num-1] = row_size;
        col_widths[enabled_col_num-1]  = 2*sizeof(time_t);
        mvcc_offset = row_size;
        row_size += 2*sizeof(time_t);
    }
    // -- pasring arguments done --------------------------------------

    int lpd_fd  = open_fd();
    struct _config* config = (struct _config *)mmap((void*)0, LPD0_SIZE, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED, lpd_fd, LPD0_ADDR);

    config->row_size = row_size;
    config->row_count = row_count;
    config->enabled_col_num = enabled_col_num;
    for(int i=0; i<enabled_col_num*2; i+=2){
      config->col_widths[i/2] = col_widths[i/2];
    }
    unsigned short sum_col_offsets = 0;
    for(int i=0; i<enabled_col_num*2; i+=2){
      config->col_offsets[i/2] = col_offsets[i/2] - sum_col_offsets;
      sum_col_offsets = col_offsets[i/2];   
    }
    config->frame_offset = frame_offset;
    
    int unmap_result = 0;
    return unmap_result;
}
