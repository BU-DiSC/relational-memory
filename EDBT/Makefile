ARM_GCC = aarch64-linux-gnu-gcc
IDIR = include
ODIR = objects
SDIR = sources_c
BDIR = sources_bash
CFLAGS = -lm  -O0 #-I$(IDIR) #-W -Wall
#-O0 don't do any optimization
#-O2 do your best optimization

all: db_generate db_config_relcache db_reset_relcache $(ODIR)/perf.o q1 q2 q3 q4 q5 db_relcache_dump q1_mvcc


db_generate: $(SDIR)/db_generate.c
	$(ARM_GCC) -o $(ODIR)/$@  $(SDIR)/$(patsubst %,%.c,$@) $(CFLAGS) -D __aarch64__

db_config_relcache: $(SDIR)/db_config_relcache.c 
	$(ARM_GCC) -o $(ODIR)/$@  $(SDIR)/$(patsubst %,%.c,$@) $(CFLAGS)

db_reset_relcache: $(SDIR)/db_reset_relcache.c 
	$(ARM_GCC) -o $(ODIR)/$@  $(SDIR)/$(patsubst %,%.c,$@) $(CFLAGS)

q1: q1_col1 q1_col2 q1_col4 q1_col8 q1_col16
q2: q2_col1 q2_col2 q2_col4 q2_col8 q2_col16
q3: q3_col1 q3_col2 q3_col4 q3_col8 q3_col16 
q4: q4_col1 q4_col2 q4_col4 q4_col8 q4_col16 
q5: q5_col1 q5_col2 q5_col4 q5_col8 q5_col16 
q1_mvcc: q1_mvcc_col4

demo: $(SDIR)/demo.c
	$(ARM_GCC) -o $(ODIR)/$@  $(SDIR)/demo.c $(CFLAGS) -D __aarch64__

q1_col1: $(SDIR)/q1.c
	$(ARM_GCC) -o $(ODIR)/$@  $(SDIR)/q1.c $(CFLAGS) -D _1BYTE $(ODIR)/perf.o -D __aarch64__

q1_col2: $(SDIR)/q1.c
	$(ARM_GCC) -o $(ODIR)/$@  $(SDIR)/q1.c $(CFLAGS) -D _2BYTE $(ODIR)/perf.o -D __aarch64__

q1_col4: $(SDIR)/q1.c
	$(ARM_GCC) -o $(ODIR)/$@  $(SDIR)/q1.c $(CFLAGS) -D _4BYTE $(ODIR)/perf.o -D __aarch64__

q1_col8: $(SDIR)/q1.c
	$(ARM_GCC) -o $(ODIR)/$@  $(SDIR)/q1.c $(CFLAGS) -D _8BYTE $(ODIR)/perf.o -D __aarch64__

q1_col16: $(SDIR)/q1.c
	$(ARM_GCC) -o $(ODIR)/$@  $(SDIR)/q1.c $(CFLAGS) -D _16BYTE $(ODIR)/perf.o -D __aarch64__

q2_col1: $(SDIR)/q2.c
	$(ARM_GCC) -o $(ODIR)/$@  $(SDIR)/q2.c $(CFLAGS) -D _1BYTE $(ODIR)/perf.o -D __aarch64__

q2_col2: $(SDIR)/q2.c
	$(ARM_GCC) -o $(ODIR)/$@  $(SDIR)/q2.c $(CFLAGS) -D _2BYTE $(ODIR)/perf.o -D __aarch64__

q2_col4: $(SDIR)/q2.c
	$(ARM_GCC) -o $(ODIR)/$@  $(SDIR)/q2.c $(CFLAGS) -D _4BYTE $(ODIR)/perf.o -D __aarch64__

q2_col8: $(SDIR)/q2.c
	$(ARM_GCC) -o $(ODIR)/$@  $(SDIR)/q2.c $(CFLAGS) -D _8BYTE $(ODIR)/perf.o -D __aarch64__

q2_col16: $(SDIR)/q2.c
	$(ARM_GCC) -o $(ODIR)/$@  $(SDIR)/q2.c $(CFLAGS) -D _16BYTE $(ODIR)/perf.o -D __aarch64__

q3_col1: $(SDIR)/q3.c
	$(ARM_GCC) -o $(ODIR)/$@  $(SDIR)/q3.c $(CFLAGS) -D _1BYTE $(ODIR)/perf.o -D __aarch64__

q3_col2: $(SDIR)/q3.c
	$(ARM_GCC) -o $(ODIR)/$@  $(SDIR)/q3.c $(CFLAGS) -D _2BYTE $(ODIR)/perf.o -D __aarch64__

q3_col4: $(SDIR)/q3.c
	$(ARM_GCC) -o $(ODIR)/$@  $(SDIR)/q3.c $(CFLAGS) -D _4BYTE $(ODIR)/perf.o -D __aarch64__

q3_col8: $(SDIR)/q3.c
	$(ARM_GCC) -o $(ODIR)/$@  $(SDIR)/q3.c $(CFLAGS) -D _8BYTE $(ODIR)/perf.o -D __aarch64__

q3_col16: $(SDIR)/q3.c
	$(ARM_GCC) -o $(ODIR)/$@  $(SDIR)/q3.c $(CFLAGS) -D _16BYTE $(ODIR)/perf.o -D __aarch64__

q4_col1: $(SDIR)/q4.c
	$(ARM_GCC) -o $(ODIR)/$@  $(SDIR)/q4.c $(CFLAGS) -D _1BYTE $(ODIR)/perf.o -D __aarch64__

q4_col2: $(SDIR)/q4.c
	$(ARM_GCC) -o $(ODIR)/$@  $(SDIR)/q4.c $(CFLAGS) -D _2BYTE $(ODIR)/perf.o -D __aarch64__

q4_col4: $(SDIR)/q4.c
	$(ARM_GCC) -o $(ODIR)/$@  $(SDIR)/q4.c $(CFLAGS) -D _4BYTE $(ODIR)/perf.o -D __aarch64__

q4_col8: $(SDIR)/q4.c
	$(ARM_GCC) -o $(ODIR)/$@  $(SDIR)/q4.c $(CFLAGS) -D _8BYTE $(ODIR)/perf.o -D __aarch64__

q4_col16: $(SDIR)/q4.c
	$(ARM_GCC) -o $(ODIR)/$@  $(SDIR)/q4.c $(CFLAGS) -D _16BYTE $(ODIR)/perf.o -D __aarch64__

q5_col1: $(SDIR)/q5.c
	$(ARM_GCC) -o $(ODIR)/$@  $(SDIR)/q5.c $(CFLAGS) -D _1BYTE $(ODIR)/perf.o -D __aarch64__

q5_col2: $(SDIR)/q5.c
	$(ARM_GCC) -o $(ODIR)/$@  $(SDIR)/q5.c $(CFLAGS) -D _2BYTE $(ODIR)/perf.o -D __aarch64__

q5_col4: $(SDIR)/q5.c
	$(ARM_GCC) -o $(ODIR)/$@  $(SDIR)/q5.c $(CFLAGS) -D _4BYTE $(ODIR)/perf.o -D __aarch64__

q5_col8: $(SDIR)/q5.c
	$(ARM_GCC) -o $(ODIR)/$@  $(SDIR)/q5.c $(CFLAGS) -D _8BYTE $(ODIR)/perf.o -D __aarch64__

q5_col16: $(SDIR)/q5.c
	$(ARM_GCC) -o $(ODIR)/$@  $(SDIR)/q5.c $(CFLAGS) -D _16BYTE $(ODIR)/perf.o -D __aarch64__

q1_mvcc_col4: $(SDIR)/q1_mvcc.c
	$(ARM_GCC) -o $(ODIR)/$@  $(SDIR)/q1_mvcc.c $(CFLAGS) -D _4BYTE $(ODIR)/perf.o -D __aarch64__


db_relcache_dump: $(SDIR)/db_relcache_dump.c
	$(ARM_GCC) -o $(ODIR)/$@  $(SDIR)/$(patsubst %,%.c,$@) $(CFLAGS)  -D _4BYTE -D __aarch64__

$(ODIR)/perf.o: $(IDIR)/performance_counters.h $(SDIR)/performance_counters.c
	$(ARM_GCC) -c -o $(ODIR)/perf.o $(SDIR)/performance_counters.c $(CFLAGS)


test:
	@echo $(OBJS)

.PHONY: clean

clean:
	rm -rf $(ODIR)/* *~ core $(SDIR)/*~ $(BDIR)/*~ 
