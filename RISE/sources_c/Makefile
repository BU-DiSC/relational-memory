GCC = gcc
IDIR = include
ODIR = objects
SDIR = ./
BDIR = sources_bash
CFLAGS = -lm  -O0 #-I$(IDIR) #-W -Wall
#-O0 don't do any optimization
#-O2 do your best optimization

all: db_reset_relcache db_config_relcache db_populate  db_sanity poke framed_q1_col4 qjoin q1_col1_perf_l1_total q1_col1_perf_l1_miss q1_col1_perf_l2_total q1_col1_perf_l2_miss q1_col2_perf_l1_total q1_col2_perf_l1_miss q1_col2_perf_l2_total q1_col2_perf_l2_miss q1_col4_perf_l1_total q1_col4_perf_l1_miss q1_col4_perf_l2_total q1_col4_perf_l2_miss q1_col8_perf_l1_total q1_col8_perf_l1_miss q1_col8_perf_l2_total q1_col8_perf_l2_miss q1_col16_perf_l1_total q1_col16_perf_l1_miss q1_col16_perf_l2_total q1_col16_perf_l2_miss

db_generate: $(SDIR)/db_generate.c
	$(GCC) -o $(ODIR)/$@  $(SDIR)/$(patsubst %,%.c,$@) $(CFLAGS) -D linux

q2: $(SDIR)/q2.c
	$(GCC) -o $(ODIR)/$@  $(SDIR)/$(patsubst %,%.c,$@) $(CFLAGS) -D linux

q3: $(SDIR)/q3.c
	$(GCC) -o $(ODIR)/$@  $(SDIR)/$(patsubst %,%.c,$@) $(CFLAGS)



test:
	@echo $(OBJS)

.PHONY: clean

clean:
	rm -rf $(ODIR)/* *~ core $(SDIR)/*~ $(BDIR)/*~ 
