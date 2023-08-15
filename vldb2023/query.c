#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#define KB            1024
#define MB            (1024*KB)

#define LPD0_ADDR     0x80000000UL
#define LPD0_SIZE     (4*KB)

#define DRAM_DB_ADDR  0x800000000UL
#define DRAM_DB2_ADDR 0x840000000UL

#define RELCACHE_ADDR 0x1000000000UL
#define RELCACHE_SIZE (2*MB)

#define MAX_GROUPS    11

#ifndef MAP_CACHE
#define MAP_CACHE 0x40
#endif

/////////////////////////////////////////// JOIN ///////////////////////////////////////////

struct ll_node {
    void *item;
    struct ll_node *next;
};

struct linked_list {
    struct ll_node *head;
};

struct ll_node *ll_allocate() {
    return malloc(sizeof(struct ll_node));
}

void ll_insert(struct linked_list *list, void *item) {
    struct ll_node *node = ll_allocate();
    node->item = item;
    node->next = list->head;
    list->head = node;
}

void ll_free(struct linked_list *list) {
    while (list->head) {
        struct ll_node *temp = list->head;
        list->head = temp->next;
        free(temp->item);
        free(temp);
    }
}

#define CAPACITY 50000 // Size of the Hash Table

// Hashtable implementation starts
size_t hash_function(uint32_t key) {
    return key % CAPACITY;
}

struct ht_item {
    uint32_t key;
    uint32_t value;
};

struct hash_table {
    struct linked_list buckets[CAPACITY];
};

struct ht_item *create_item(uint32_t key, const uint32_t value) {
    struct ht_item *item = malloc(sizeof(struct ht_item));
    item->key = key;
    item->value = value;
    return item;
}

struct hash_table *ht_create() {
    struct hash_table *ht = malloc(sizeof(struct hash_table));
    for (int i = 0; i < CAPACITY; i++) {
        ht->buckets[i].head = NULL;
    }
    return ht;
}

void ht_free(struct hash_table *ht) {
    for (int i = 0; i < CAPACITY; i++) {
        ll_free(&ht->buckets[i]);
    }
    free(ht);
}

void ht_insert(struct hash_table *ht, uint32_t key, uint32_t value) {
    struct ht_item *item = create_item(key, value);
    size_t index = hash_function(key);
    ll_insert(&ht->buckets[index], item);
}

int ht_search(struct hash_table *ht, uint32_t key, uint32_t *value) {
    size_t index = hash_function(key);
    struct ll_node *node = ht->buckets[index].head;

    while (node != NULL) {
        struct ht_item *item = node->item;
        if (item->key == key) {
            *value = item->value;
            return 1;
        }
        node = node->next;
    }
    return 0;
}

// Hashtable implementation finish

struct config {
    unsigned int row_size;        // 4B
    unsigned int row_count;       // 4B
    unsigned int reset;           // 4B

    // We support maximum of 11 columns
    unsigned int enabled_col_num; // 4B
    unsigned short col_widths[MAX_GROUPS];  // 22B
    unsigned short col_offsets[MAX_GROUPS]; // 22B

    unsigned int frame_offset;    // 4B
}; // 64B

#ifdef __aarch64__
#define dsb() asm volatile("dsb 15")
#ifdef CLOCK
#include <time.h>
#else

clock_t clock() {
    clock_t val;
    asm volatile("mrs %0, cntvct_el0" : "=r" (val));
    return val;
}

#endif
#else
#define dsb() do {} while(0)

#include <time.h>

#endif

#define SEL_COL 0
#define JOIN_COL 1

struct table {
    uint32_t row_count;
    uint32_t row_size;
    uint8_t num_cols;
    uint8_t *widths;
};

struct avrg_args {
    struct table s;
    uint8_t col;
};

typedef enum {
    O_LT, O_LTE, O_GT, O_GTE, O_EQ, O_NE, O_NOP
} op_t;

struct slct_args {
    struct table s;
    uint8_t num_cols;
    struct {
        uint8_t col;
        uint8_t project;
        op_t op;
        uint32_t k;
    } cols[MAX_GROUPS];
};

struct join_args {
    struct table s;
    struct table r;
    uint8_t s_sel;
    uint8_t r_sel;
    uint8_t s_join;
    uint8_t r_join;
};

typedef enum {
    S_ROW, S_COL, S_RME
} store_t;

typedef enum {
    Q_AVRG, Q_SLCT, Q_JOIN
} query_t;

struct arguments {
    store_t store;
    query_t query;
    union {
        struct avrg_args avrg;
        struct slct_args slct;
        struct join_args join;
    };
};

struct config *config;
void *plim;
size_t db_size;
void *db;
size_t db2_size;
void *db2;
int verbose;

void logger(const char *format, ...) {
    if (verbose > 0) {
        va_list args;
        va_start(args, format);
        vfprintf(stderr, format, args);
        va_end(args);
    }
}

size_t calc_offset(struct table *t, uint8_t col) {
    size_t offset = 0;
    for (int i = 0; i < col; i++) {
        offset += t->widths[i];
    }
    return offset;
}

void *memmap(size_t len, off_t offset, unsigned long flag) {
#ifdef __aarch64__
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd == -1) {
        fprintf(stderr, "Can't open /dev/mem.\n");
        exit(EXIT_FAILURE);
    }
    void *ptr = mmap(NULL, len, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_SHARED | flag, fd, offset);
    if (ptr == MAP_FAILED) {
        fprintf(stderr, "Error: mmap");
        exit(EXIT_FAILURE);
    }
    close(fd);
    return ptr;
#else
    return malloc(len);
#endif
}

void memunmap(void *ptr, size_t len) {
#ifdef __aarch64__
    munmap(ptr, len);
#else
    free(ptr);
#endif
}

void db_reset(unsigned int frame_offset) {
    dsb();
    config->frame_offset = frame_offset;
    config->reset = (config->reset + 1) & 0x1;
    dsb();
}

void db_config(struct arguments *args) {
    if (args->store != S_RME) return;

    config = memmap(LPD0_SIZE, LPD0_ADDR, 0);
    switch (args->query) {
        case Q_AVRG: {
            config->row_size = args->avrg.s.row_size;
            config->row_count = args->avrg.s.row_count + 64;
            config->enabled_col_num = 1;
            config->col_offsets[0] = calc_offset(&args->avrg.s, args->avrg.col);
            config->col_widths[0] = args->avrg.s.widths[args->avrg.col];
            break;
        }
        case Q_SLCT: {
            config->row_size = args->slct.s.row_size;
            config->row_count = args->slct.s.row_count + 64;
            config->enabled_col_num = args->slct.num_cols;
            config->col_offsets[0] = calc_offset(&args->slct.s, args->slct.cols[0].col);
            config->col_widths[0] = args->slct.s.widths[args->slct.cols[0].col];
            unsigned short prev_abs_offset = config->col_offsets[0];
            for (int i = 1; i < args->slct.num_cols; ++i) {
                unsigned short abs_offset = calc_offset(&args->slct.s, args->slct.cols[i].col);
                config->col_offsets[i] = abs_offset - prev_abs_offset;
                config->col_widths[i] = args->slct.s.widths[args->slct.cols[i].col];
                prev_abs_offset = abs_offset;
            }
            break;
        }
        case Q_JOIN: {
            config->row_size = args->join.s.row_size;
            config->row_count = args->join.s.row_count + 64;
            config->enabled_col_num = 2;
            config->col_widths[SEL_COL] = args->join.s.widths[args->join.s_sel];
            config->col_widths[JOIN_COL] = args->join.s.widths[args->join.s_join];
            config->col_offsets[SEL_COL] = calc_offset(&args->join.s, args->join.s_sel);
            config->col_offsets[JOIN_COL] = calc_offset(&args->join.s, args->join.s_join);
            break;
        }
    }

    config->frame_offset = 0x0;
    db_reset(0x0);
    logger("Config:\n");
    for (int i = 0; i < config->enabled_col_num; ++i) {
        logger("\toffset: %3hu width: %hu\n", config->col_offsets[i], config->col_widths[i]);
    }
    plim = memmap(RELCACHE_SIZE, RELCACHE_ADDR, MAP_CACHE);
}

void db2_config(struct arguments *args) {
    config->row_size = args->join.r.row_size;
    config->row_count = args->join.r.row_count + 64;
    config->enabled_col_num = 2;
    config->col_offsets[SEL_COL] = calc_offset(&args->join.r, args->join.r_sel);
    config->col_widths[SEL_COL] = args->join.r.widths[args->join.r_sel];
    config->col_offsets[JOIN_COL] = calc_offset(&args->join.r, args->join.r_join);
    config->col_widths[JOIN_COL] = args->join.r.widths[args->join.r_join];
    config->frame_offset = 0x0;
    db_reset(0x40000000);

    logger("Config:\n");
    for (int i = 0; i < config->enabled_col_num; ++i) {
        logger("\toffset: %3hu width: %hu\n", config->col_offsets[i], config->col_widths[i]);
    }
    plim = memmap(RELCACHE_SIZE, RELCACHE_ADDR, MAP_CACHE);
}

void db_populate_col(struct table *t, void *data) {
    for (int i = 0; i < t->row_count; ++i) {
        size_t col_offset = 0;
        for (int j = 0; j < t->num_cols; ++j) {
            uint64_t val = 100 * i + j;
            size_t offset = col_offset + i * t->widths[j];
            if (t->widths[j] == 1) {
                *(uint8_t *) (data + offset) = val;
            } else if (t->widths[j] == 2) {
                *(uint16_t *) (data + offset) = val;
            } else if (t->widths[j] == 4) {
                *(uint32_t *) (data + offset) = val;
            } else if (t->widths[j] == 8) {
                *(uint64_t *) (data + offset) = val;
            }
            col_offset += t->widths[j] * t->row_count;
        }
    }
}

void db_populate_row(struct table *t, void *data) {
    size_t offset = 0;
    for (int i = 0; i < t->row_count; ++i) {
        for (int j = 0; j < t->num_cols; ++j) {
            uint64_t val = 100 * i + j;
            if (t->widths[j] == 1) {
                *(uint8_t *) (data + offset) = val;
            } else if (t->widths[j] == 2) {
                *(uint16_t *) (data + offset) = val;
            } else if (t->widths[j] == 4) {
                *(uint32_t *) (data + offset) = val;
            } else if (t->widths[j] == 8) {
                *(uint64_t *) (data + offset) = val;
            }
            offset += t->widths[j];
        }
    }
}

void db_init(struct arguments *args) {
    struct table *s;
    switch (args->query) {
        case Q_AVRG:
            s = &args->avrg.s;
            break;
        case Q_SLCT:
            s = &args->slct.s;
            break;
        case Q_JOIN:
            s = &args->join.s;
            break;
    }

    // db generate
    db_size = s->row_count * s->row_size;
    logger("Allocating memory (%luB)\n", db_size);
    db = memmap(db_size, DRAM_DB_ADDR, 0);

    if (args->store == S_COL) {
        logger("Populating column store\n");
        db_populate_col(s, db);
    } else {
        logger("Populating row store\n");
        db_populate_row(s, db);

        if (args->store == S_RME) {
            logger("Configuring RME\n");
            db_config(args);
        }
    }
#ifdef __aarch64__
    memunmap(db, db_size);
    db = memmap(db_size, DRAM_DB_ADDR, 0);
#endif
}

void db2_init(struct arguments *args) {
    struct table *r = &args->join.r;

    // db generate
    db2_size = r->row_count * r->row_size;
    logger("Allocating memory (%luB)\n", db2_size);
    db2 = memmap(db2_size, DRAM_DB2_ADDR, 0);

    if (args->store == S_COL) {
        logger("Populating column store\n");
        db_populate_col(r, db2);
    } else {
        logger("Populating row store\n");
        db_populate_row(r, db2);

        if (args->store == S_RME) {
            logger("Configuring RME\n");
            db2_config(args);
        }
    }
#ifdef __aarch64__
    memunmap(db2, db2_size);
    db2 = memmap(db2_size, DRAM_DB2_ADDR, 0);
#endif
}

void parse_table_args(char **argv, struct table *t) {
    logger("Table:\n");
    t->row_count = strtoul(argv[0], NULL, 0);
    t->num_cols = strtoul(argv[1], NULL, 0);
    t->widths = malloc(t->num_cols * sizeof(uint8_t));
    t->row_size = 0;
    logger("\tRows: %u\n", t->row_count);
    logger("\tCols: %hhu\n", t->num_cols);
    logger("\tWidths:");
    char *pt = strtok(argv[2], ",");
    for (int i = 0; i < t->num_cols; i++) {
        if (pt == NULL) {
            fprintf(stderr, "Error: too few column widths\n");
            exit(EXIT_FAILURE);
        }
        t->widths[i] = strtoul(pt, NULL, 0);
        logger(" %hhu", t->widths[i]);
        t->row_size += t->widths[i];
        pt = strtok(NULL, ",");
    }
    logger("\n\tRow size: %u\n", t->row_size);
}

void parse_table_args2(char **argv, struct table *t) {
    logger("Table:\n");
    t->row_count = strtoul(argv[0], NULL, 0);
    t->num_cols = strtoul(argv[1], NULL, 0);
    t->widths = malloc(t->num_cols * sizeof(uint8_t));
    t->row_size = 0;
    logger("\tRows: %u\n", t->row_count);
    logger("\tCols: %hhu\n", t->num_cols);
    logger("\tWidths:");
    char *pt = strtok(argv[2], ",");
    for (int i = 0; i < t->num_cols; i++) {
        if (pt == NULL) {
            fprintf(stderr, "Error: too few column widths\n");
            exit(EXIT_FAILURE);
        }
//        t->widths[i] = strtoul(pt, NULL, 0);
        t->widths[i] = sizeof(uint32_t);
        logger(" %hhu", t->widths[i]);
        t->row_size += t->widths[i];
        pt = strtok(NULL, ",");
    }
    logger("\n\tRow size: %u\n", t->row_size);
}

void free_args(struct arguments *args) {
    switch (args->query) {
        case Q_AVRG:
            free(args->avrg.s.widths);
            break;
        case Q_SLCT:
            free(args->slct.s.widths);
            break;
        case Q_JOIN:
            free(args->join.s.widths);
            free(args->join.r.widths);
            break;
    }
}

void parse_args(int argc, char **argv, struct arguments *args) {
    if (strcmp(argv[1], "-q") == 0) {
        argv++;
        argc--;
        verbose = 0;
    } else {
        verbose = 1;
    }
    char *store = argv[1];
    if (strcmp(store, "ROW") == 0) {
        args->store = S_ROW;
        logger("ROW store\n");
    } else if (strcmp(store, "COL") == 0) {
        args->store = S_COL;
        logger("COL store\n");
    } else if (strcmp(store, "RME") == 0) {
        args->store = S_RME;
        logger("RME store\n");
    } else {
        fprintf(stderr, "Error: unknown store %s\n", store);
        exit(EXIT_FAILURE);
    }

    char *query = argv[2];
    if (strcmp(query, "q1") == 0) { // argc 7
        if (argc != 7) {
            fprintf(stderr, "Error: unexpected args %d\n", argc);
            exit(EXIT_FAILURE);
        }
        logger("AVG query\n");
        args->query = Q_AVRG;
        parse_table_args(&argv[3], &args->avrg.s);
        args->avrg.col = strtoul(argv[6], NULL, 0);
        logger("target column: %hhu\n", args->avrg.col);
    } else if (strcmp(query, "q2") == 0) { // argc 8
        if (argc != 8) {
            fprintf(stderr, "Error: unexpected args %d\n", argc);
            exit(EXIT_FAILURE);
        }
        logger("SELECT query\n");
        args->query = Q_SLCT;
        parse_table_args(&argv[3], &args->slct.s);
        args->slct.num_cols = strtoul(argv[6], NULL, 0);
        if (args->slct.num_cols > 11) {
            fprintf(stderr, "Error: maximum number of columns 11\n");
            exit(EXIT_FAILURE);
        }
        char *pt = strtok(argv[7], ",");
        for (int i = 0; i < args->slct.num_cols; ++i) {
            if (pt == NULL) {
                fprintf(stderr, "Error: too few columns\n");
                exit(EXIT_FAILURE);
            }
            char *end;
            args->slct.cols[i].col = strtoul(pt, &end, 0);
            args->slct.cols[i].project = *end == 'P';
            end++;
            if (*end == 'L') {
                args->slct.cols[i].op = O_LT;
            } else if (*end == 'l') {
                args->slct.cols[i].op = O_LTE;
            } else if (*end == 'G') {
                args->slct.cols[i].op = O_GT;
            } else if (*end == 'g') {
                args->slct.cols[i].op = O_GTE;
            } else if (*end == 'E') {
                args->slct.cols[i].op = O_EQ;
            } else if (*end == 'N') {
                args->slct.cols[i].op = O_NE;
            } else {
                args->slct.cols[i].op = O_NOP;
            }
            args->slct.cols[i].k = strtoul(end + 1, NULL, 0);
            pt = strtok(NULL, ",");
        }
    } else if (strcmp(query, "q3") == 0) { // argc 13
        if (argc != 13) {
            fprintf(stderr, "Error: unexpected args %d\n", argc);
            exit(EXIT_FAILURE);
        }
        logger("JOIN query\n");
        args->query = Q_JOIN;
        parse_table_args2(&argv[3], &args->join.s);
        parse_table_args2(&argv[6], &args->join.r);
        args->join.s_sel = strtoul(argv[9], NULL, 0);
        args->join.r_sel = strtoul(argv[10], NULL, 0);
        args->join.s_join = strtoul(argv[11], NULL, 0);
        args->join.r_join = strtoul(argv[12], NULL, 0);
        logger("select s column: %hhu\n", args->join.s_sel);
        logger("select r column: %hhu\n", args->join.r_sel);
        logger("join s column: %hhu\n", args->join.s_join);
        logger("join s column: %hhu\n", args->join.r_join);
    } else {
        fprintf(stderr, "Error: unknown query %s\n", query);
        exit(EXIT_FAILURE);
    }
}

uint8_t avg_uint8_row(void *data, uint32_t row_count, uint32_t row_size) {
    uint32_t sum = 0;
    for (int i = 0; i < row_count; i++) {
        sum += *(uint8_t *) data;
        data += row_size;
    }
    return sum / row_count;
}

uint16_t avg_uint16_row(void *data, uint32_t row_count, uint32_t row_size) {
    uint32_t sum = 0;
    for (int i = 0; i < row_count; i++) {
        sum += *(uint16_t *) data;
        data += row_size;
    }
    return sum / row_count;
}

uint32_t avg_uint32_row(void *data, uint32_t row_count, uint32_t row_size) {
    uint32_t sum = 0;
    for (int i = 0; i < row_count; i++) {
        sum += *(uint32_t *) data;
        data += row_size;
    }
    return sum / row_count;
}

uint64_t avg_uint64_row(void *data, uint32_t row_count, uint32_t row_size) {
    uint64_t sum = 0;
    for (int i = 0; i < row_count; i++) {
        sum += *(uint64_t *) data;
        data += row_size;
    }
    return sum / row_count;
}

void avg_row(struct arguments *args) {
    uint32_t offset = 0;
    for (int i = 0; i < args->avrg.col; ++i) {
        offset += args->avrg.s.widths[i];
    }
    clock_t start = 0, end = 0;
    uint64_t res = 0;
    if (args->avrg.s.widths[args->avrg.col] == 1) {
        start = clock();
        res = avg_uint8_row(db + offset, args->avrg.s.row_count, args->avrg.s.row_size);
        end = clock();
    } else if (args->avrg.s.widths[args->avrg.col] == 2) {
        start = clock();
        res = avg_uint16_row(db + offset, args->avrg.s.row_count, args->avrg.s.row_size);
        end = clock();
    } else if (args->avrg.s.widths[args->avrg.col] == 4) {
        start = clock();
        res = avg_uint32_row(db + offset, args->avrg.s.row_count, args->avrg.s.row_size);
        end = clock();
    } else if (args->avrg.s.widths[args->avrg.col] == 8) {
        start = clock();
        res = avg_uint64_row(db + offset, args->avrg.s.row_count, args->avrg.s.row_size);
        end = clock();
    }
    logger("Result rows: 1\n");
    logger("Execution time: %lu\n", end - start);
    logger("Avg(c%hhu)\n", args->avrg.col);
    logger("%lu\n", res);
    fprintf(stdout, "%lu\n", end - start);
}

uint8_t avg_uint8_col(const uint8_t *col, uint32_t row_count) {
    uint32_t sum = 0;
    for (int i = 0; i < row_count; i++) {
        sum += col[i];
    }
    return sum / row_count;
}

uint16_t avg_uint16_col(const uint16_t *col, uint32_t row_count) {
    uint32_t sum = 0;
    for (int i = 0; i < row_count; i++) {
        sum += col[i];
    }
    return sum / row_count;
}

uint32_t avg_uint32_col(const uint32_t *col, uint32_t row_count) {
    uint32_t sum = 0;
    for (int i = 0; i < row_count; i++) {
        sum += col[i];
    }
    return sum / row_count;
}

uint64_t avg_uint64_col(const uint64_t *col, uint32_t row_count) {
    uint64_t sum = 0;
    for (int i = 0; i < row_count; i++) {
        sum += col[i];
    }
    return sum / row_count;
}

void avg_col(struct arguments *args) {
    size_t offset = 0;
    for (int i = 0; i < args->avrg.col; ++i) {
        offset += args->avrg.s.widths[i] * args->avrg.s.row_count;
    }
    uint64_t res = 0;
    clock_t start = 0, end = 0;
    if (args->avrg.s.widths[args->avrg.col] == 1) {
        start = clock();
        res = avg_uint8_col(db + offset, args->avrg.s.row_count);
        end = clock();
    } else if (args->avrg.s.widths[args->avrg.col] == 2) {
        start = clock();
        res = avg_uint16_col(db + offset, args->avrg.s.row_count);
        end = clock();
    } else if (args->avrg.s.widths[args->avrg.col] == 4) {
        start = clock();
        res = avg_uint32_col(db + offset, args->avrg.s.row_count);
        end = clock();
    } else if (args->avrg.s.widths[args->avrg.col] == 8) {
        start = clock();
        res = avg_uint64_col(db + offset, args->avrg.s.row_count);
        end = clock();
    }
    logger("Result rows: 1\n");
    logger("Execution time: %lu\n", end - start);
    logger("Avg(c%hhu)\n", args->avrg.col);
    logger("Result rows: %lu\n", res);
    fprintf(stdout, "%lu\n", end - start);
}

void avg_rme(struct arguments *args) {
    uint64_t res = 0;
    clock_t start = 0, end = 0;
    if (args->avrg.s.widths[args->avrg.col] == 1) {
        start = clock();
        res = avg_uint8_col(plim, args->avrg.s.row_count);
        end = clock();
    } else if (args->avrg.s.widths[args->avrg.col] == 2) {
        start = clock();
        res = avg_uint16_col(plim, args->avrg.s.row_count);
        end = clock();
    } else if (args->avrg.s.widths[args->avrg.col] == 4) {
        start = clock();
        res = avg_uint32_col(plim, args->avrg.s.row_count);
        end = clock();
    } else if (args->avrg.s.widths[args->avrg.col] == 8) {
        start = clock();
        res = avg_uint64_col(plim, args->avrg.s.row_count);
        end = clock();
    }
    logger("Result rows: 1\n");
    logger("Execution time: %lu\n", end - start);
    logger("Avg(c%hhu)\n", args->avrg.col);
    logger("Result rows: %lu\n", res);
    fprintf(stdout, "%lu\n", end - start);
}

void avg(struct arguments *args) {
    switch (args->store) {
        case S_ROW:
            avg_row(args);
            break;
        case S_COL:
            avg_col(args);
            break;
        case S_RME:
            avg_rme(args);
            break;
    }
}

void slct_row(struct arguments *args) {
    size_t offsets[MAX_GROUPS];
    uint8_t conditions[MAX_GROUPS];
    uint8_t projections[MAX_GROUPS];
    uint8_t condition_count = 0;
    uint8_t projection_count = 0;
    for (int j = 0; j < args->slct.num_cols; ++j) {
        offsets[j] = calc_offset(&args->slct.s, args->slct.cols[j].col);
        if (args->slct.cols[j].op != O_NOP) {
            conditions[condition_count] = j;
            condition_count++;
        }
        if (args->slct.cols[j].project) {
            projections[projection_count] = j;
            projection_count++;
        }
    }

    uint32_t res_count = 0;
    void *result = malloc(db_size);
    void *ptr = result;
    void *row = db;
    clock_t start = clock();
    for (int i = 0; i < args->slct.s.row_count; ++i) {
        uint8_t bad = 0;
        for (int j = 0; j < condition_count; ++j) {
            int col = conditions[j];
            if (args->slct.cols[col].op == O_LT) {
                uint8_t col_width = args->slct.s.widths[args->slct.cols[col].col];
                if (col_width == 1) {
                    uint8_t value = *(uint8_t *) (row + offsets[col]);
                    if (value >= args->slct.cols[col].k) {
                        bad = 1;
                        break;
                    }
                } else if (col_width == 2) {
                    uint16_t value = *(uint16_t *) (row + offsets[col]);
                    if (value >= args->slct.cols[col].k) {
                        bad = 1;
                        break;
                    }
                } else if (col_width == 4) {
                    uint32_t value = *(uint32_t *) (row + offsets[col]);
                    if (value >= args->slct.cols[col].k) {
                        bad = 1;
                        break;
                    }
                } else if (col_width == 8) {
                    uint64_t value = *(uint64_t *) (row + offsets[col]);
                    if (value >= args->slct.cols[col].k) {
                        bad = 1;
                        break;
                    }
                }
            }
        }

        if (bad) continue;
        for (int j = 0; j < projection_count; ++j) {
            int col = projections[j];
            uint8_t width = args->slct.s.widths[args->slct.cols[col].col];
            memcpy(ptr, row + offsets[col], width);
            ptr += width;
        }
        res_count++;
        row += args->slct.s.row_size;
    }
    clock_t end = clock();

    logger("Result rows: %u\n", res_count);
    logger("Execution time: %lu\n", end - start);
    for (int j = 0; j < projection_count; ++j) {
        int col = projections[j];
        logger("c%hhu ", args->slct.cols[col].col);
    }
    logger("\n");
    ptr = result;
    for (int i = 0; i < res_count; ++i) {
        for (int j = 0; j < projection_count; ++j) {
            int col = projections[j];
            uint8_t width = args->slct.s.widths[args->slct.cols[col].col];
            switch (width) {
                case 1: {
                    uint8_t val = *(uint8_t *) ptr;
                    logger("%hhu ", val);
                    break;
                }
                case 2: {
                    uint16_t val = *(uint16_t *) ptr;
                    logger("%hu ", val);
                    break;
                }
                case 4: {
                    uint32_t val = *(uint32_t *) ptr;
                    logger("%u ", val);
                    break;
                }
                case 8: {
                    uint64_t val = *(uint64_t *) ptr;
                    logger("%lu ", val);
                    break;
                }
                default: {
                    break;
                }
            }
            ptr += width;
        }
        logger("\n");
    }
    free(result);
    fprintf(stdout, "%lu\n", end - start);
}

void slct_col(struct arguments *args) {
    void *cols[MAX_GROUPS];
    uint8_t conditions[MAX_GROUPS];
    uint8_t projections[MAX_GROUPS];
    uint8_t condition_count = 0;
    uint8_t projection_count = 0;
    for (int j = 0; j < args->slct.num_cols; ++j) {
        cols[j] = db + calc_offset(&args->slct.s, args->slct.cols[j].col) * args->slct.s.row_count;
        if (args->slct.cols[j].op != O_NOP) {
            conditions[condition_count] = j;
            condition_count++;
        }
        if (args->slct.cols[j].project) {
            projections[projection_count] = j;
            projection_count++;
        }
    }

    uint32_t res_count = 0;
    void *result = malloc(db_size);
    void *ptr = result;
    clock_t start = clock();
    for (int i = 0; i < args->slct.s.row_count; ++i) {
        uint8_t bad = 0;
        for (int j = 0; j < condition_count; ++j) {
            int col = conditions[j];
            if (args->slct.cols[col].op == O_LT) {
                uint8_t col_width = args->slct.s.widths[args->slct.cols[col].col];
                if (col_width == 1) {
                    uint8_t value = ((uint8_t *) cols[col])[i];
                    if (value >= args->slct.cols[col].k) {
                        bad = 1;
                        break;
                    }
                } else if (col_width == 2) {
                    uint16_t value = ((uint16_t *) cols[col])[i];
                    if (value >= args->slct.cols[col].k) {
                        bad = 1;
                        break;
                    }
                } else if (col_width == 4) {
                    uint32_t value = ((uint32_t *) cols[col])[i];
                    if (value >= args->slct.cols[col].k) {
                        bad = 1;
                        break;
                    }
                } else if (col_width == 8) {
                    uint64_t value = ((uint64_t *) cols[col])[i];
                    if (value >= args->slct.cols[col].k) {
                        bad = 1;
                        break;
                    }
                }
            }
        }

        if (bad) continue;
        for (int j = 0; j < projection_count; ++j) {
            int col = projections[j];
            if (args->slct.cols[col].project) {
                uint8_t col_width = args->slct.s.widths[args->slct.cols[col].col];
                memcpy(ptr, cols[col] + i * col_width, col_width);
                ptr += col_width;
            }
        }
        res_count++;
    }
    clock_t end = clock();
    logger("Result rows: %u\n", res_count);
    logger("Execution time: %lu\n", end - start);
    for (int j = 0; j < projection_count; ++j) {
        int col = projections[j];
        logger("c%hhu ", args->slct.cols[col].col);
    }
    logger("\n");
    ptr = result;
    for (int i = 0; i < res_count; ++i) {
        for (int j = 0; j < projection_count; ++j) {
            int col = projections[j];
            uint8_t width = args->slct.s.widths[args->slct.cols[col].col];
            switch (width) {
                case 1: {
                    uint8_t val = *(uint8_t *) ptr;
                    logger("%hhu ", val);
                    break;
                }
                case 2: {
                    uint16_t val = *(uint16_t *) ptr;
                    logger("%hu ", val);
                    break;
                }
                case 4: {
                    uint32_t val = *(uint32_t *) ptr;
                    logger("%u ", val);
                    break;
                }
                case 8: {
                    uint64_t val = *(uint64_t *) ptr;
                    logger("%lu ", val);
                    break;
                }
                default: {
                    break;
                }
            }
            ptr += width;
        }
        logger("\n");
    }
    free(result);
    fprintf(stdout, "%lu\n", end - start);
}

void slct_rme(struct arguments *args) {
    uint8_t conditions[MAX_GROUPS];
    uint8_t projections[MAX_GROUPS];
    uint8_t condition_count = 0;
    uint8_t projection_count = 0;
    size_t plim_row_size = 0;
    for (int j = 0; j < args->slct.num_cols; ++j) {
        if (args->slct.cols[j].op != O_NOP) {
            conditions[condition_count] = j;
            condition_count++;
        }
        if (args->slct.cols[j].project) {
            projections[projection_count] = j;
            projection_count++;
        }
        plim_row_size += args->slct.s.widths[args->slct.cols[j].col];
    }

    uint32_t res_count = 0;
    void *result = malloc(db_size);
    void *ptr = result;
    void *row = plim;
    clock_t start = clock();
    for (int i = 0; i < args->slct.s.row_count; ++i) {
        uint8_t bad = 0;
        for (int j = 0; j < condition_count; ++j) {
            int col = conditions[j];
            if (args->slct.cols[col].op == O_LT) {
                uint8_t col_width = config->col_widths[col];
                if (col_width == 1) {
                    uint8_t value = *(uint8_t *) (row + config->col_offsets[col]);
                    if (value >= args->slct.cols[col].k) {
                        bad = 1;
                        break;
                    }
                } else if (col_width == 2) {
                    uint16_t value = *(uint16_t *) (row + config->col_offsets[col]);
                    if (value >= args->slct.cols[col].k) {
                        bad = 1;
                        break;
                    }
                } else if (col_width == 4) {
                    uint32_t value = *(uint32_t *) (row + config->col_offsets[col]);
                    if (value >= args->slct.cols[col].k) {
                        bad = 1;
                        break;
                    }
                } else if (col_width == 8) {
                    uint64_t value = *(uint64_t *) (row + config->col_offsets[col]);
                    if (value >= args->slct.cols[col].k) {
                        bad = 1;
                        break;
                    }
                }
            }
        }
        if (bad) continue;

        for (int j = 0; j < projection_count; ++j) {
            int col = projections[j];
            if (args->slct.cols[col].project) {
                uint8_t width = args->slct.s.widths[args->slct.cols[col].col];
                memcpy(ptr, row + config->col_offsets[col], width);
                ptr += width;
            }
        }
        res_count++;
        row += plim_row_size;
    }
    clock_t end = clock();
    logger("Result rows: %u\n", res_count);
    logger("Execution time: %lu\n", end - start);
    for (int j = 0; j < projection_count; ++j) {
        int col = projections[j];
        logger("c%hhu ", args->slct.cols[col].col);
    }
    logger("\n");
    ptr = result;
    for (int i = 0; i < res_count; ++i) {
        for (int j = 0; j < projection_count; ++j) {
            int col = projections[j];
            uint8_t width = args->slct.s.widths[args->slct.cols[col].col];
            switch (width) {
                case 1: {
                    uint8_t val = *(uint8_t *) ptr;
                    logger("%hhu ", val);
                    break;
                }
                case 2: {
                    uint16_t val = *(uint16_t *) ptr;
                    logger("%hu ", val);
                    break;
                }
                case 4: {
                    uint32_t val = *(uint32_t *) ptr;
                    logger("%u ", val);
                    break;
                }
                case 8: {
                    uint64_t val = *(uint64_t *) ptr;
                    logger("%lu ", val);
                    break;
                }
                default: {
                    break;
                }
            }
            ptr += width;
        }
        logger("\n");
    }
    free(result);
    fprintf(stdout, "%lu\n", end - start);
}

void slct(struct arguments *args) {
    switch (args->store) {
        case S_ROW:
            slct_row(args);
            break;
        case S_COL:
            slct_col(args);
            break;
        case S_RME:
            slct_rme(args);
            break;
    }
}

void join_row(struct arguments *args) {
    uint32_t sum_access = 0;
    uint32_t sum_hash_creation = 0;

    clock_t start, end;
    struct hash_table *ht = ht_create();
    // ************************* Dram Hash Table1 Starts *****************************
    uint32_t *row = db;
    for (int i = 0; i < args->join.s.row_count; i++) {
        start = clock();
        uint32_t key = row[args->join.s_join];
        uint32_t val = row[args->join.s_sel];
        row += args->join.s.num_cols;
        end = clock();
        sum_access += end - start;

        start = clock();
        ht_insert(ht, key, val);
        end = clock();
        sum_hash_creation += end - start;
    }
    // ************************* Dram Hash Table1 Ends *****************************

    //************************************* POPULATE 2 *******************************************
    db2_init(args);

    // ************************* PLIM Hash Table2 Starts *****************************
    // Assuming maximum of 2 matching per col1 of first Table. The other 2 is for keeping all columns from BOTH tables
    uint32_t *join_result = malloc(2 * sizeof(uint32_t) * args->join.r.row_count);
    uint32_t res_count = 0;
    row = db2;

    // ************************* DRAM Hash Table2 Starts *****************************
    // Assuming maximum of 2 matching per col1 of first Table. The other 2 is for keeping all columns from BOTH tables
    // magic_timing_begin(&cycleLo, &cycleHi);
    for (int i = 0; i < args->join.r.row_count; i++) {
        start = clock();
        uint32_t key = row[args->join.r_join];
        uint32_t val2 = row[args->join.r_sel];
        row += args->join.r.num_cols;
        end = clock();
        sum_access += end - start;

        start = clock();
        uint32_t val1;
        int found = ht_search(ht, key, &val1);
        end = clock();
        sum_hash_creation += end - start;

        if (!found) {
            continue;
        }
        join_result[res_count] = val1;
        res_count++;
        join_result[res_count] = val2;
        res_count++;
    }
    ht_free(ht);

    logger("Result rows: %u\n", res_count / 2);
    logger("Execution time: %u\n", sum_access);

    logger("S.c%hhu R.c%hhu\n", args->join.s_sel, args->join.r_sel);
    for (int i = 0; i < res_count; i+=2) {
        logger("%u %u\n", join_result[i], join_result[i+1]);
    }
    free(join_result);
    fprintf(stdout, "%u\n", sum_access);
}

void join_col(struct arguments *args) {
    uint32_t sum_access = 0;
    uint32_t sum_hash_creation = 0;

    clock_t start, end;
    struct hash_table *ht = ht_create();
    // ************************* Dram Hash Table1 Starts *****************************
    size_t offset = calc_offset(&args->join.s, args->join.s_join) * args->join.s.row_count;
    uint32_t *key_col = db + offset;
    offset = calc_offset(&args->join.s, args->join.s_sel) * args->join.s.row_count;
    uint32_t *val_col = db + offset;
    for (int i = 0; i < args->join.s.row_count; i++) {
        start = clock();
        uint32_t key = key_col[i];
        uint32_t val = val_col[i];
        end = clock();
        sum_access += end - start;

        start = clock();
        ht_insert(ht, key, val);
        end = clock();
        sum_hash_creation += end - start;
    }
    // ************************* Dram Hash Table1 Ends *****************************

    //************************************* POPULATE 2 *******************************************
    db2_init(args);

    // ************************* PLIM Hash Table2 Starts *****************************
    // Assuming maximum of 2 matching per col1 of first Table. The other 2 is for keeping all columns from BOTH tables
    uint32_t *join_result = malloc(2 * sizeof(uint32_t) * args->join.r.row_count);
    uint32_t res_count = 0;
    offset = calc_offset(&args->join.r, args->join.r_join) * args->join.r.row_count;
    key_col = db2 + offset;
    offset = calc_offset(&args->join.r, args->join.r_sel) * args->join.r.row_count;
    val_col = db2 + offset;

    // ************************* DRAM Hash Table2 Starts *****************************
    // Assuming maximum of 2 matching per col1 of first Table. The other 2 is for keeping all columns from BOTH tables
    // magic_timing_begin(&cycleLo, &cycleHi);
    for (int i = 0; i < args->join.r.row_count; i++) {
        start = clock();
        uint32_t key = key_col[i];
        uint32_t val2 = val_col[i];
        end = clock();
        sum_access += end - start;

        start = clock();
        uint32_t val1;
        int found = ht_search(ht, key, &val1);
        end = clock();
        sum_hash_creation += end - start;

        if (!found) {
            continue;
        }
        join_result[res_count] = val1;
        res_count++;
        join_result[res_count] = val2;
        res_count++;
    }
    ht_free(ht);

    logger("Result rows: %u\n", res_count / 2);
    logger("Execution time: %u\n", sum_access);
    logger("S.c%hhu R.c%hhu\n", args->join.s_sel, args->join.r_sel);
    for (int i = 0; i < res_count; i+=2) {
        logger("%u %u\n", join_result[i], join_result[i+1]);
    }
    free(join_result);
    fprintf(stdout, "%u\n", sum_access);
}

void join_rme(struct arguments *args) {
    uint32_t sum_access = 0;
    uint32_t sum_hash_creation = 0;

    clock_t start, end;
    struct hash_table *ht = ht_create();
    uint32_t *ptr = plim;
    for (int i = 0; i < args->join.s.row_count; i++) {
        start = clock();
        uint32_t key = *ptr;
        ptr++;
        uint32_t val = *ptr;
        ptr++;
        end = clock();
        sum_access += end - start;
        if (args->join.s_sel < args->join.s_join) {
            uint32_t temp = key;
            key = val;
            val = temp;
        }

        start = clock();
        ht_insert(ht, key, val);
        end = clock();
        sum_hash_creation += end - start;
    }
    // ************************* PLIM Hash Table1 Ends *****************************
    memunmap(plim, RELCACHE_SIZE);

//    for (int i = 0; i < 10; ++i) {
//        struct ll_node *node = ht->buckets[i].head;
//        if (node) {
//            struct ht_item *item = node->item;
//            logger("> %u %u\n", item->key, item->value);
//        } else {
//            logger("> -\n");
//        }
//    }
    //************************************* POPULATE 2 *******************************************
    db2_init(args);

    // ************************* PLIM Hash Table2 Starts *****************************
    // Assuming maximum of 2 matching per col1 of first Table. The other 2 is for keeping all columns from BOTH tables
    uint32_t *join_result = malloc(2 * sizeof(uint32_t) * args->join.r.row_count);
    uint32_t res_count = 0;
    ptr = plim;
    for (int i = 0; i < args->join.r.row_count; i++) {
        start = clock();
        uint32_t key = *ptr;
        ptr++;
        uint32_t val2 = *ptr;
        ptr++;
        end = clock();
        sum_access += end - start;
        if (args->join.r_sel < args->join.r_join) {
            uint32_t temp = key;
            key = val2;
            val2 = temp;
        }

        start = clock();
        uint32_t val1;
        int found = ht_search(ht, key, &val1);
        end = clock();
        sum_hash_creation += end - start;

        if (!found) {
            continue;
        }
//        logger("Match %u\n", key);
        join_result[res_count] = val1;
        res_count++;
        join_result[res_count] = val2;
        res_count++;
    }
    ht_free(ht);

    logger("Result rows: %u\n", res_count / 2);
    logger("Execution time: %u\n", sum_access);
    logger("S.c%hhu R.c%hhu\n", args->join.s_sel, args->join.r_sel);
    for (int i = 0; i < res_count; i+=2) {
        logger("%u %u\n", join_result[i], join_result[i+1]);
    }
    free(join_result);
    fprintf(stdout, "%u\n", sum_access);
}

void join(struct arguments *args) {
    switch (args->store) {
        case S_ROW:
            join_row(args);
            break;
        case S_COL:
            join_col(args);
            break;
        case S_RME:
            join_rme(args);
            break;
    }
}

int main(int argc, char **argv) {
    struct arguments args;
    parse_args(argc, argv, &args);

    db_init(&args);

    // do something
    logger("Running\n");
    switch (args.query) {
        case Q_AVRG:
            avg(&args);
            break;
        case Q_SLCT:
            slct(&args);
            break;
        case Q_JOIN:
            join(&args);
            break;
    }

    if (args.store == S_RME) {
        db_reset(0x0);
        if (args.query == Q_JOIN) {
            db_reset(0x40000000);
        }
        memunmap(plim, RELCACHE_SIZE);
        memunmap(config, LPD0_SIZE);
    }
    memunmap(db, db_size);
    if (args.query == Q_JOIN) {
        memunmap(db2, db2_size);
    }
    free_args(&args);
    return 0;
}
