#define _GNU_SOURCE
#include "query_header.h"
#include "q5_header.h"

int main(int argc, char** argv) {
    unsigned int row_size;
    unsigned int row_count;
    unsigned int enabled_col_num;
    unsigned short col_widths[MAX_GROUPS];//We support maximum of 11 columns
    unsigned short col_offsets[MAX_GROUPS];
    unsigned int   frame_offset = 0;
    unsigned short   sum_col_widths = 0;

    unsigned int cycleHi    = 0, cycleLo=0;

    unsigned int sum_plim = 0;
    unsigned int sum_direct = 0;
    unsigned int hashing_phase = 0;
    unsigned int probing_phase = 0;
    unsigned int sum_plim_access = 0;
    unsigned int sum_plim_hash_creation = 0;

    unsigned int sum_dram_access = 0;
    unsigned int sum_dram_hash_creation = 0;

    // -- pasring arguments -------------------------------------------
    int opt;
    while ((opt = getopt(argc, argv, "C:R:W:r:O:F:")) != -1) {
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

    unsigned int reset_data = 0;
    unsigned dram_size  = row_count*row_size;
    unsigned db_size    = row_count*row_size;

    int dram_populate_fd   = open_fd();
    int dram_populate_fd2  = open_fd();
    int lpd_fd             = open_fd();
    int hpm_fd             = open_fd();
    int hpm_fd2            = open_fd();
    int dram_fd            = open_fd();
    int dram_fd2           = open_fd();
    
    //Uncached mapping
    struct _config* config = (struct _config *)mmap((void*)0, LPD0_SIZE, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED, lpd_fd, LPD0_ADDR);
    T* db = mmap((void*)0, db_size+frame_offset, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED, dram_populate_fd, HIGH_DDR_ADDR); 
    T* plim   = mmap((void*)0, RELCACHE_SIZE, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED|0x40, hpm_fd, RELCACHE_ADDR);
    T* dram   = mmap((void*)0, dram_size+frame_offset, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED|0x40, dram_fd, HIGH_DDR_ADDR);
    T *result1 = (T *)malloc(row_count * enabled_col_num * sizeof(T));
    T *result2 = (T *)malloc(row_count * enabled_col_num * sizeof(T));
    T data;

    //************************************* POPULATE 1 *******************************************
    for(int i=0; i< (db_size)/sizeof(T); i++){
        db[i] = 0;
    }
    
    unsigned int base = 1;
    for(int i=0; i < row_count; i++){
      for(int j=0; j < enabled_col_num; j++){
        db[ ((i*row_size) + col_offsets[j])/sizeof(T) ] = base;
        base++;
      }      
    }
    
    //************************************* CONFIG *******************************************
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
    reset_data = config->reset;
    // printf("Configuration complete\n");
    __dsb();
    config->frame_offset = 0x0;  
    config->reset = (++reset_data) & 0x1;
    __dsb();

    // ************************* PLIM Hash Table1 Starts *****************************
    // magic_timing_begin(&cycleLo, &cycleHi);
    // printf("hashing table1.....\n");
    HashTable* ht = create_table(CAPACITY);
    T *arr1;    
    arr1=(T*)malloc(sizeof(T)*(enabled_col_num - 1));
    // printf("Created.\n");
    for (int i = 0; i < row_count; i++)
    {
      for (int j = 0; j < enabled_col_num; j++) {
        if (j == 0)
          continue;
        // *(arr1 + j - 1) = *(result1 + i*enabled_col_num + j);
        magic_timing_begin(&cycleLo, &cycleHi);
        *(arr1 + j - 1) = plim[i*enabled_col_num + j];
        magic_timing_end(&cycleLo, &cycleHi);
        sum_plim_access += cycleLo;
        // printf("%x", *(result1 + i*enabled_col_num + j)); 
      }
      // printf("%u\n", *(result1 + i*enabled_col_num));
      // ht_insert(ht, *(result1 + i*enabled_col_num), arr1, enabled_col_num - 1);
      magic_timing_begin(&cycleLo, &cycleHi);
      data = plim[i*enabled_col_num];
      magic_timing_end(&cycleLo, &cycleHi);
      sum_plim_access += cycleLo;

      magic_timing_begin(&cycleLo, &cycleHi);
      ht_insert(ht, data, arr1, enabled_col_num - 1);
      magic_timing_end(&cycleLo, &cycleHi);
      sum_plim_hash_creation += cycleLo;
    }
    // magic_timing_end(&cycleLo, &cycleHi);
    sum_plim += cycleLo;
    hashing_phase = sum_plim;
    // ************************* PLIM Hash Table1 Ends *****************************


    // ************************* Dram Hash Table1 Starts *****************************
    HashTable* ht2 = create_table(CAPACITY);
    T *arr2;
    arr2=(T*)malloc(row_size - col_widths[0]);

    for (int i = 0; i < row_count; i++)
    {
      for (int j = 0; j < enabled_col_num; j++) {
        if (j == 0)
          continue;
        magic_timing_begin(&cycleLo, &cycleHi);
        *(arr2 + j - 1) = dram[((i*row_size) + col_offsets[j])/sizeof(T)];
        magic_timing_end(&cycleLo, &cycleHi);
        sum_dram_access += cycleLo;
      }
        magic_timing_begin(&cycleLo, &cycleHi);
        data = dram[(i*row_size)/sizeof(T)];
        magic_timing_end(&cycleLo, &cycleHi);
        sum_dram_access += cycleLo;

        magic_timing_begin(&cycleLo, &cycleHi);
        ht_insert(ht2, data, arr2, enabled_col_num - 1);
        magic_timing_end(&cycleLo, &cycleHi);
        sum_dram_hash_creation += cycleLo;
    }
    sum_direct += cycleLo;
    // ************************* Dram Hash Table1 Ends *****************************

   //************************************* UNMAPPING - REMAPPING*************************************
   munmap(db, db_size+frame_offset);
   munmap(plim, RELCACHE_SIZE);
   munmap(dram, dram_size+frame_offset);
   close((int)dram_populate_fd);
   close((int)hpm_fd);
   close((int)dram_fd);
   
   T* db2 = mmap((void*)0, db_size+frame_offset, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED, dram_populate_fd2, HIGH_DDR_ADDR2); 
   T* plim2   = mmap((void*)0, RELCACHE_SIZE, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED|0x40, hpm_fd2, RELCACHE_ADDR);
   T* dram2  = mmap((void*)0, dram_size+frame_offset, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED|0x40, dram_fd2, HIGH_DDR_ADDR2);

   //************************************* RESET *******************************************
   __dsb();
   config->frame_offset = 0x40000000;  
   config->reset = (++reset_data) & 0x1;
   __dsb();

    
   //************************************* POPULATE 2 *******************************************
   // printf("\nPopulating DB for second table...\n");
   for(int i=0; i < (db_size)/sizeof(T); i++){
       db2[i] = 0;
   }

   base = 1;
   unsigned int base2 = 100000;
   for(int i=0; i < row_count; i++){
     for(int j=0; j < enabled_col_num; j++){
        if (j == 0)
        {
          db2[ ((i*row_size) + col_offsets[j])/sizeof(T) ] = base;
          base += 4;
        }
        else
        {
          db2[ ((i*row_size) + col_offsets[j])/sizeof(T) ] = base2;
          base2++;
        }
     }      
   }

  // ************************* PLIM Hash Table2 Starts *****************************
    // Assuming maximum of 2 matching per col1 of first Table. The other 2 is for keeping all columns from BOTH tables
   magic_timing_begin(&cycleLo, &cycleHi);
    T *join_result = (T *)malloc(2 * row_count * 2 * enabled_col_num * sizeof(T));
    int match_count = 0;

    T* val;
    for (int i = 0; i < row_count; i++)
    {
        magic_timing_begin(&cycleLo, &cycleHi);
        data = plim[i*enabled_col_num];
        magic_timing_end(&cycleLo, &cycleHi);
        sum_plim_access += cycleLo;

        magic_timing_begin(&cycleLo, &cycleHi);
        val = ht_search(ht, data);
        magic_timing_end(&cycleLo, &cycleHi);
        sum_plim_hash_creation += cycleLo;

      if (val == NULL) {
        continue;
      }
      else
      {
        magic_timing_begin(&cycleLo, &cycleHi);
        *(join_result + match_count * 2 * enabled_col_num) = plim[i*enabled_col_num];
        *(join_result + match_count * 2 * enabled_col_num + enabled_col_num) = plim[i*enabled_col_num];
        for (int j = 0; j < (enabled_col_num - 1); j++)
        {
          *(join_result + match_count * 2 * enabled_col_num + j + 1) = *(val + j);
          *(join_result + match_count * 2 * enabled_col_num + enabled_col_num + j + 1) = plim[i*enabled_col_num + j + 1];
        }
        magic_timing_end(&cycleLo, &cycleHi);
        sum_plim_access += cycleLo;
        match_count++;
      }
    }
    magic_timing_end(&cycleLo, &cycleHi);
    sum_plim += cycleLo;
    printf("q5, r, %d, %d, %d, %d, %d\n", row_size, row_count, col_widths[0], sum_plim_access, sum_plim_hash_creation);


    // ************************* DRAM Hash Table2 Starts *****************************
    // Assuming maximum of 2 matching per col1 of first Table. The other 2 is for keeping all columns from BOTH tables
    // magic_timing_begin(&cycleLo, &cycleHi);
    T *join_result2 = (T *)malloc(2 * row_count * 2 * enabled_col_num * sizeof(T));
    int match_count2 = 0;
    // T* val;
    // magic_timing_begin(&cycleLo, &cycleHi);
    for (int i = 0; i < row_count; i++)
    {
        magic_timing_begin(&cycleLo, &cycleHi);
        data = dram2[(i*row_size)/sizeof(T)];
        magic_timing_end(&cycleLo, &cycleHi);
        sum_dram_access += cycleLo;

        magic_timing_begin(&cycleLo, &cycleHi);
        val = ht_search(ht2, data);
        magic_timing_end(&cycleLo, &cycleHi);
        sum_dram_hash_creation += cycleLo;

      if (val == NULL) {
        continue;
      }
      else
      {
        magic_timing_begin(&cycleLo, &cycleHi);
        *(join_result2 + match_count2 * 2 * enabled_col_num) = dram2[(i*row_size)/sizeof(T)];
        *(join_result2 + match_count2 * 2 * enabled_col_num + enabled_col_num) = dram2[(i*row_size)/sizeof(T)];
        for (int j = 0; j < (enabled_col_num - 1); j++)
        {
          *(join_result2 + match_count2 * 2 * enabled_col_num + j + 1) = *(val + j);
          *(join_result2 + match_count2 * 2 * enabled_col_num + enabled_col_num + j + 1) = dram2[((i*row_size) + col_offsets[j+1])/sizeof(T)];
        }
        magic_timing_end(&cycleLo, &cycleHi);
        sum_dram_access += cycleLo;
        match_count2++;
      }
    }
    // magic_timing_end(&cycleLo, &cycleHi);
    sum_direct += cycleLo;
    
    printf("q5, d, %d, %d, %d, %d, %d\n", row_size, row_count, col_widths[0], sum_dram_access, sum_dram_hash_creation);

    return 0;
}
