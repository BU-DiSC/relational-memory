#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define CL_SIZE        64

#define K              1000
#define KB             1024
#define MB             1024*KB

#define RELCACHE_ADDR  0x1000000000UL
#define RELCACHE_SIZE  2*MB
#define DRAM_ADDR      0x800000000UL
#define OCM_ADDR       0x00FFFC0000
#define OCM_SIZE       256*KB

#define    LPD0_SIZE  4*KB
#define    LPD0_ADDR  0x80000000

#ifdef _1BYTE
	#define T    unsigned char                  //1Byte  
#elif _2BYTE
	#define T    unsigned short                 //2Byte
#elif _4BYTE
	#define T    unsigned int                   //4Byte
#elif _8BYTE
	#define T    unsigned long int              //8Byte
#elif _16BYTE
	#define T    __uint128_t        //16Byte
#else
	#define T    unsigned int                   //4Byte
#endif

#ifdef linux
  #define magic_timing_begin(cycleLo, cycleHi){\
    *cycleHi=0;\
    *cycleLo=0;\
  }
  #define magic_timing_end(cycleLo, cycleHi){\
    *cycleHi=0;\
    *cycleLo=0;\
  }
#else
  #define magic_timing_begin(cycleLo, cycleHi){\
    *cycleHi=0;\
    asm volatile("mrs %0, CNTVCT_EL0": "=r"(*cycleLo) );\
  }\
  
  #define magic_timing_end(cycleLo, cycleHi){\
    unsigned tempCycleLo, tempCycleHi =0;\
    asm volatile("mrs %0, CNTVCT_EL0":"=r"(tempCycleLo) );\
    *cycleLo = tempCycleLo - *cycleLo;\
    *cycleHi = tempCycleHi - *cycleHi;\
  }
#endif

int open_fd() {
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd == -1) {
        printf("Can't open /dev/mem.\n");
        exit(0);
    }
    return fd;
}

#define MAX_GROUPS 11
struct _config {
  unsigned int   row_size; //4B
  unsigned int   row_count; //4B
  unsigned int   reset;//4B
  unsigned int   enabled_col_num;//4B
  unsigned short col_widths[MAX_GROUPS];//We support maximum of 11 columns //22B
  unsigned short col_offsets[MAX_GROUPS];//22B
  unsigned int   frame_offset;//4B
}; 

