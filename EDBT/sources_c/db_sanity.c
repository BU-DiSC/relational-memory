#include "query_header.h"

int main(int argc, char** argv) {
    unsigned int row_size;
    unsigned int row_count;
    unsigned int enabled_col_num;
    unsigned short col_widths[11];//We support maximum of 11 columns
    unsigned short col_offsets[11];
    unsigned int   frame_offset = 0;
    unsigned short   sum_col_widths = 0;


    sscanf(argv[1], "%ud", &row_size);
    sscanf(argv[2], "%ud", &row_count);
    sscanf(argv[3], "%ud", &enabled_col_num);
    for(int i=0; i<enabled_col_num*2; i+=2){
      sscanf(argv[4+i], "%hu", &col_widths[i/2]);
      sscanf(argv[4+i+1], "%hu", &col_offsets[i/2]);
      sum_col_widths += col_widths[i/2];
    }
    sscanf(argv[4+enabled_col_num*2], "%ud", &frame_offset);
    printf("----frame offset: %u\n",frame_offset);
    unsigned dram_size  = row_count*row_size;
    int hpm_fd          = open_fd();
    int dram_fd         = open_fd();

    //mapping fpga:
    unsigned char* plim = mmap((void*)0, RELCACHE_SIZE, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED|0x40, hpm_fd, RELCACHE_ADDR);
    //mapping dram
    unsigned char* dram = mmap((void*)0, dram_size+frame_offset, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED|0x40, dram_fd, DRAM_ADDR);

    //printing plim char by char
    for(int i = 0; i < row_count; i++){
      printf("\n row %d  ",i);
      printf("plim : ");
      for(int j=0; j<enabled_col_num; j++)
        for(int k=0; k<col_widths[j]; k++)
          printf("%02x", plim[i*sum_col_widths + k]);
      printf("     dram : ");
      for(int j=0; j<enabled_col_num; j++)
        for(int k=0; k<col_widths[j]; k++)
          printf("%02x", dram[i*row_size + col_offsets[j] + k + (unsigned int)frame_offset]);
    }
    printf("\n");

    return 0;
}