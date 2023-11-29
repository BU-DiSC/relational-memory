# Define the default C compiler for aarch64
CC = aarch64-linux-gnu-gcc

# Define the output name for the compiled program
OUTPUT = new_demo

# Define where the header files are located
INCLUDE_DIR = ./include

# Define where the utility source files are located
UTILS_DIR = ./utils

QUERY_DIR = ./queries

# All the .c source files
SOURCES = demo.c \
          $(UTILS_DIR)/db_generate.c \
          $(UTILS_DIR)/db_relcache.c \
          $(UTILS_DIR)/exp_config.c \
          $(UTILS_DIR)/parse_config.c \
          $(UTILS_DIR)/performance_counters.c \
          $(UTILS_DIR)/print_utils.c \
          $(QUERY_DIR)/q4.c\
          $(QUERY_DIR)/q1.c\
          $(QUERY_DIR)/q2.c\
          $(QUERY_DIR)/q3.c\
          $(QUERY_DIR)/experiments.c

# Compilation flags (adding -lm and -O0)
# Added -D __aarch64__ for aarch64 specific compilation
CFLAGS = -I$(INCLUDE_DIR) -lm -O0 -D __aarch64__

# The default target (typing 'make' will use this target)
all: $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(OUTPUT)

# Clean target to remove output and intermediate files
clean:
	rm -f $(OUTPUT) *.o

