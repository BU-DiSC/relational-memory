#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#define KB            1024
#define MB            (1024*KB)

#define LPD0_ADDR     0x000080000000UL
#define LPD0_SIZE     (4*KB)

#define DRAM_DB_ADDR  0x000800000000UL
#define DRAM_DB2_ADDR 0x000840000000UL

#define RELCACHE_ADDR 0x001000000000UL
#define RELCACHE_SIZE (2*MB)

#define MAX_GROUPS    11

#ifndef MAP_32BIT
#define MAP_32BIT 0x40
#endif

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

int open_fd() {
#ifdef __aarch64__
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
#else
    int fd = 0;
#endif
    if (fd == -1) {
        printf("Can't open /dev/mem.\n");
        exit(0);
    }
    return fd;
}

#ifdef __aarch64__
#define dsb() {asm volatile("dsb 15");}
#ifdef CLOCK
#include <time.h>
#else
uint64_t clock() {
    uint64_t val;
    asm volatile("mrs %0, cntvct_el0" : "=r" (val));
    return val;
}
#endif
#else
#define dsb() {}

#include <time.h>

#endif


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
    O_LT,
    O_LTE,
    O_GT,
    O_GTE,
    O_EQ,
    O_NE,
    O_NOP
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
    S_ROW,
    S_COL,
    S_RME
} store_t;

typedef enum {
    Q_AVRG,
    Q_SLCT,
    Q_JOIN
} query_t;

struct experiment {
    store_t store;
    query_t query;
    union {
        struct avrg_args avrg;
        struct slct_args slct;
        struct join_args join;
    };
};

struct config *config;

int config_fd;
int plim_fd;
int db_fd;
int db2_fd;
size_t db_size;
void *db;
size_t db2_size;
void *db2;
void *plim;

void calc_offset_width(struct table *t, uint8_t col, unsigned short *offset, unsigned short *width) {
    unsigned short o = 0;
    for (int i = 0; i < col; i++) {
        o += t->widths[i];
    }
    *width = t->widths[col];
    *offset = o;
}

void db_config(struct experiment *args) {
    if (args->store != S_RME) return;

    config_fd = open_fd();
    config = mmap(NULL, LPD0_SIZE, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_SHARED | MAP_32BIT, config_fd, LPD0_ADDR);
    switch (args->query) {
        case Q_AVRG:
            config->row_size = args->avrg.s.row_size;
            config->row_count = args->avrg.s.row_count;
            config->enabled_col_num = 1;
            calc_offset_width(&args->avrg.s, args->avrg.col, config->col_offsets, config->col_widths);
            break;
        case Q_SLCT:
            config->row_size = args->slct.s.row_size;
            config->row_count = args->slct.s.row_count;
            config->enabled_col_num = args->slct.num_cols;
            for (int i = 0; i < args->slct.num_cols; ++i) {
                calc_offset_width(&args->slct.s, args->slct.cols[i].col, config->col_offsets + i,
                                  config->col_widths + i);
            }
            break;
        case Q_JOIN:
            config->row_size = args->join.s.row_size;
            config->row_count = args->join.s.row_count;
            config->enabled_col_num = 2;
            calc_offset_width(&args->join.s, args->join.s_sel, config->col_offsets, config->col_widths);
            calc_offset_width(&args->join.s, args->join.s_join, config->col_offsets + 1, config->col_widths + 1);
            break;
    }

    config->frame_offset = 0x0;

    fprintf(stderr, "Config:\n");
    for (int i = 0; i < config->enabled_col_num; ++i) {
        fprintf(stderr, "\toffset: %3hu width: %hu\n", config->col_offsets[i], config->col_widths[i]);
    }
//    if (args.query == Q_JOIN) {
//        size_t db2_size = args->join.r.row_count * args->join.r.row_size;
//        db2 = mmap(NULL, db2_size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_SHARED | MAP_32BIT, mem_fd, DRAM_DB2_ADDR);
//    }
#ifdef __aarch64__
    plim_fd = open_fd();
    plim = mmap(NULL, RELCACHE_SIZE, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_SHARED | MAP_32BIT, plim_fd, RELCACHE_ADDR);
#else
    plim = malloc(RELCACHE_SIZE);
#endif
}

void db_reset(unsigned int frame_offset) {
    dsb();
    config->frame_offset = frame_offset;
    config->reset = (config->reset + 1) & 0x1;
    dsb();
    config->frame_offset = frame_offset;
    config->reset = (config->reset + 1) & 0x1;
    dsb();
}

void parse_table_args(char **argv, struct table *t) {
    fprintf(stderr, "Table:\n");
    t->row_count = strtoul(argv[0], NULL, 0);
    t->num_cols = strtoul(argv[1], NULL, 0);
    t->widths = malloc(t->num_cols * sizeof(uint8_t));
    t->row_size = 0;
    fprintf(stderr, "\tRows: %d\n", t->row_count);
    fprintf(stderr, "\tCols: %d\n", t->num_cols);
    fprintf(stderr, "\tWidths:");
    char *pt = strtok(argv[2], ",");
    for (int i = 0; i < t->num_cols; i++) {
        if (pt == NULL) {
            printf("Error: too few column widths\n");
            exit(EXIT_FAILURE);
        }
        t->widths[i] = strtoul(pt, NULL, 0);
        fprintf(stderr, " %d", t->widths[i]);
        t->row_size += t->widths[i];
        pt = strtok(NULL, ",");
    }
    fprintf(stderr, "\n\tRow size: %d\n", t->row_size);
}

void free_args(struct experiment *args) {
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

void parse_args(int argc, char **argv, struct experiment *args) {
    char *store = argv[1];
    if (strcmp(store, "ROW") == 0) {
        args->store = S_ROW;
        fprintf(stderr, "ROW store\n");
    } else if (strcmp(store, "COL") == 0) {
        args->store = S_COL;
        fprintf(stderr, "COL store\n");
    } else if (strcmp(store, "RME") == 0) {
        args->store = S_RME;
        fprintf(stderr, "RME store\n");
    } else {
        printf("Error: unknown store %s\n", store);
        exit(EXIT_FAILURE);
    }

    char *query = argv[2];
    if (strcmp(query, "q1") == 0) { // argc 7
        if (argc != 7) {
            printf("Error: unexpected args %d\n", argc);
            exit(EXIT_FAILURE);
        }
        fprintf(stderr, "AVG query\n");
        args->query = Q_AVRG;
        parse_table_args(&argv[3], &args->avrg.s);
        args->avrg.col = strtoul(argv[6], NULL, 0);
        fprintf(stderr, "target column: %d\n", args->avrg.col);
    } else if (strcmp(query, "q2") == 0) { // argc 8
        if (argc != 8) {
            printf("Error: unexpected args %d\n", argc);
            exit(EXIT_FAILURE);
        }
        fprintf(stderr, "SELECT query\n");
        args->query = Q_SLCT;
        parse_table_args(&argv[3], &args->slct.s);
        args->slct.num_cols = strtoul(argv[6], NULL, 0);
        if (args->slct.num_cols > 11) {
            printf("Error: maximum number of columns 11\n");
            exit(EXIT_FAILURE);
        }
        char *pt = strtok(argv[7], ",");
        for (int i = 0; i < args->slct.num_cols; ++i) {
            if (pt == NULL) {
                printf("Error: too few columns\n");
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
            printf("Error: unexpected args %d\n", argc);
            exit(EXIT_FAILURE);
        }
        fprintf(stderr, "JOIN query\n");
        args->query = Q_JOIN;
        parse_table_args(&argv[3], &args->join.s);
        parse_table_args(&argv[6], &args->join.r);
        args->join.s_sel = strtoul(argv[9], NULL, 0);
        args->join.r_sel = strtoul(argv[10], NULL, 0);
        args->join.s_join = strtoul(argv[11], NULL, 0);
        args->join.r_join = strtoul(argv[12], NULL, 0);
        fprintf(stderr, "select s column: %d\n", args->join.s_sel);
        fprintf(stderr, "select r column: %d\n", args->join.r_sel);
        fprintf(stderr, "join s column: %d\n", args->join.s_join);
        fprintf(stderr, "join s column: %d\n", args->join.r_join);
    } else {
        printf("Error: unknown query %s\n", query);
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

void avg_row(struct experiment *args) {
    uint32_t offset = 0;
    for (int i = 0; i < args->avrg.col; ++i) {
        offset += args->avrg.s.widths[i];
    }
    uint64_t start = 0, end = 0;
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
    fprintf(stderr, "Result: %lu\n", res);
    fprintf(stderr, "%lu\n", end - start);
    printf("%lu\n", end - start);
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

void avg_col(struct experiment *args) {
    size_t offset = 0;
    for (int i = 0; i < args->avrg.col; ++i) {
        offset += args->avrg.s.widths[i] * args->avrg.s.row_count;
    }
    uint64_t res = 0;
    uint64_t start = 0, end = 0;
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
    fprintf(stderr, "Result: %lu\n", res);
    fprintf(stderr, "%lu\n", end - start);
    printf("%lu\n", end - start);
}

void avg_rme(struct experiment *args) {
    uint64_t res = 0;
    uint64_t start = 0, end = 0;
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
    fprintf(stderr, "Result: %lu\n", res);
    fprintf(stderr, "%lu\n", end - start);
    printf("%lu\n", end - start);
}

void avg(struct experiment *args) {
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


void slct_row(struct experiment *args) {
//    uint32_t offset = 0;
//    for (int i = 0; i < args->slct.num_cols; ++i) {
//#if 0
//        switch (args->slct.cols[i].op) {
//            case O_LT:
//                break;
//            case O_LTE:
//                break;
//            case O_GT:
//                break;
//            case O_GTE:
//                break;
//            case O_EQ:
//                break;
//            case O_NE:
//                break;
//            case O_NOP:
//                break;
//        }
//#endif
//        args->slct.cols[i].col
//        if ()
//        offset += args->slct.s.widths[i];
//    }
//    uint64_t start = 0, end = 0;
//    uint64_t res = 0;
//    if (args->slct.s.widths[args->slct.col] == 1) {
//        start = clock();
//        res = avg_uint8_row(db + offset, args->slct.s.row_count, args->slct.s.row_size);
//        end = clock();
//    } else if (args->slct.s.widths[args->slct.col] == 2) {
//        start = clock();
//        res = avg_uint16_row(db + offset, args->slct.s.row_count, args->slct.s.row_size);
//        end = clock();
//    } else if (args->slct.s.widths[args->slct.col] == 4) {
//        start = clock();
//        res = avg_uint32_row(db + offset, args->slct.s.row_count, args->slct.s.row_size);
//        end = clock();
//    } else if (args->slct.s.widths[args->slct.col] == 8) {
//        start = clock();
//        res = avg_uint64_row(db + offset, args->slct.s.row_count, args->slct.s.row_size);
//        end = clock();
//    }
//    fprintf(stderr, "Result: %lu\n", res);
//    fprintf(stderr, "%lu\n", end - start);
//    printf("%lu\n", end - start);
}

void slct(struct experiment *args) {
//    switch (args->store) {
//        case S_ROW:
//            slct_row(args);
//            break;
//        case S_COL:
//            slct_col(args);
//            break;
//        case S_RME:
//            slct_rme(args);
//            break;
//    }
}

void db_populate_col(struct table *t, void *data) {
    for (int i = 0; i < t->row_count; ++i) {
        size_t col_offset = 0;
        for (int j = 0; j < t->num_cols; ++j) {
            uint64_t val = j;
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
            uint64_t val = j;
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

int main(int argc, char **argv) {
    struct experiment args;
    parse_args(argc, argv, &args);

    struct table *s;
    struct table *r;
    switch (args.query) {
        case Q_AVRG:
            s = &args.avrg.s;
            r = NULL;
            break;
        case Q_SLCT:
            s = &args.slct.s;
            r = NULL;
            break;
        case Q_JOIN:
            s = &args.join.s;
            r = &args.join.r;
            break;
    }

    // db generate
    db_size = s->row_count * s->row_size;
    fprintf(stderr, "Allocating memory (%luB)\n", db_size);
#ifdef __aarch64__
    db_fd = open_fd();
    db = mmap(NULL, db_size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_SHARED | MAP_32BIT, db_fd, DRAM_DB_ADDR);
#else
    db = malloc(db_size);
#endif
    if (args.store == S_COL) {
        fprintf(stderr, "Populating column store\n");
        db_populate_col(s, db);
    } else {
        fprintf(stderr, "Populating row store\n");
        db_populate_row(s, db);

        if (args.store == S_RME) {
            fprintf(stderr, "Configuring RME\n");
            db_config(&args);
        }
    }

    // do something
    fprintf(stderr, "Running\n");
    switch (args.query) {
        case Q_AVRG:
            avg(&args);
            break;
        case Q_SLCT:
            slct(&args);
            break;
        case Q_JOIN:
            break;
    }

    if (args.store == S_RME) {
        db_reset(0x0);
//        db_reset(0x40000000);
#ifdef __aarch64__
        munmap(plim, RELCACHE_SIZE);
#else
        free(plim);
#endif
    }
#ifdef __aarch64__
    close(db_fd);
    munmap(db, db_size);
#else
    free(db);
#endif
    free_args(&args);
    return 0;
}
