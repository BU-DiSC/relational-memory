/** @file performance_counters.c
 * @ingroup base
 * @brief Implementation of a architecture independent and highly abstrcat way access to performance counters.
 * @details Uses interface provided by the Linux kernel to access teh performance counters value.
 * @author Denis Hoornaert
 */

#include <stdio.h>
#include <linux/perf_event.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "../include/performance_counters.h"

/// System-call number to open performance counter event.
#define __NR_perf_event_open 241

/// Core model specific performance counter event IDs
#define        L1_REFERENCES 0x04
#define           L1_REFILLS 0x03
#define        L2_REFERENCES 0x16
#define           L2_REFILLS 0x17
#define         INST_RETIRED 0x08

/// Indicates which thread/process performance counters to follow.
#define          this_thread 0

/// Enables monitoring of the task performance events on any cores
#define             any_core -1

/** @brief Struct holding raw measurement and ID of a performance counter.
 *
 */
struct event {
        long unsigned value;         /* The value of the event */
        long unsigned id;            /* if PERF_FORMAT_ID */
};

/** @brief Struct returned by the kernel upon reading the file descriptor of the performance counters.
 *  @detail The struct holds values for l1-D refills and misses and l2 refills and misses.
 */
struct read_format {
        long unsigned nr;            /* The number of events */
        long unsigned time_enabled;  /* if PERF_FORMAT_TOTAL_TIME_ENABLED */
        long unsigned time_running;  /* if PERF_FORMAT_TOTAL_TIME_RUNNING */
        struct event l1_references;
        struct event l1_refills;
        struct event l2_references;
        struct event l2_refills;
        struct event inst_retired;
};

/// File descriptor for L1-D references (also, group-fd head)
static int l1_references_fd;

/// File descriptor for L1-D missess
static int l1_refills_fd;

/// File descriptor for L2 references
static int l2_references_fd;

// File descriptor for L2 misses
static int l2_refills_fd;

/// File descriptor for instruction retired
static int inst_retired_fd;


/**
 * @brief Open a file descriptor for the performance counter specified.
 * @param[in] pmc_type The platform specific ID of the performance counter.
 * @param[in] group_fd The file descriptor group to which the performance counter belongs.
 * @param[in] this_cpu The CPU to which the core is attached.
 * @return The file directory opened, -1 on failures.
 */
static int open_pmc_fd(unsigned int pmc_type, int group_fd)
{
	static struct perf_event_attr attr;
	attr.type = PERF_TYPE_RAW;
	attr.config = pmc_type;
	attr.size = sizeof(struct perf_event_attr);
	attr.read_format = PERF_FORMAT_GROUP|PERF_FORMAT_ID|PERF_FORMAT_TOTAL_TIME_ENABLED|PERF_FORMAT_TOTAL_TIME_RUNNING;
    attr.disabled = 0;
    attr.exclude_kernel = 1;

	int fd = syscall(__NR_perf_event_open, &attr, this_thread, any_core, group_fd, 0);

	if (fd == -1) {
		perror("Could not open fd for performance counter\n");
	}

	return fd;
}

/** @brief Enable user-space access to performance counters.
 * @return Group_fd head's pid on sucess, -1 on error.
 */
int setup_pmcs(void)
{
	l1_references_fd = open_pmc_fd(L1_REFERENCES, -1);
    if (l1_references_fd == -1)
        return -1;
	l1_refills_fd = open_pmc_fd(L1_REFILLS, l1_references_fd);
    if (l1_refills_fd == -1)
        return -1;
	l2_references_fd = open_pmc_fd(L2_REFERENCES, l1_references_fd);
    if (l2_references_fd == -1)
        return -1;
	l2_refills_fd = open_pmc_fd(L2_REFILLS, l1_references_fd);
    if (l2_refills_fd == -1)
        return -1;
    inst_retired_fd = open_pmc_fd(INST_RETIRED, l1_references_fd);
    if (inst_retired_fd == -1)
            return -1;
	return l1_references_fd;
}

/**
 * @brief Close the file descriptor related to the performance counters.
 * @param[in] fd The file descriptor to close.
 * @param[in] pmc_type The platform specific ID of the performance counter to close.
 * @return Returns file descriptor status upon closing, return -1 on failures.
 */
static inline int close_pmc_fd(int fd)
{
	int ret = close(fd);
	if (ret == -1) {
		perror("Could not close fd for performance counter\n");
	}
	return ret;
}

/** @brief Close access to performance counters.
 * @return 0 on sucess, -1 on error.
 */
int teardown_pmcs(void)
{
	int ret = 0;
	ret = close_pmc_fd(l1_references_fd);
	if (ret == -1)
		return ret;
	ret = close_pmc_fd(l1_refills_fd);
    if (ret == -1)
        return ret;
	ret = close_pmc_fd(l2_references_fd);
    if (ret == -1)
        return ret;
	ret = close_pmc_fd(l2_refills_fd);
    if (ret == -1)
        return ret;
    ret = close_pmc_fd(inst_retired_fd);
    if (ret == -1)
        return ret;
	return 0;
}

/** @brief Read performance counters value.
 * @return struct perf_countrers.
 */
void pmcs_get_value(struct perf_counters* res)
{
	struct read_format measurement;
	size_t size = read(l1_references_fd, &measurement, sizeof(struct read_format));
	if (size != sizeof(struct read_format)) {
		perror("Error: Size read from performance counters differ from size expected.");
	}
	res->l1_references = measurement.l1_references.value;
	res->l1_refills = measurement.l1_refills.value;
	res->l2_references = measurement.l2_references.value;
	res->l2_refills = measurement.l2_refills.value;
    res->inst_retired = measurement.inst_retired.value;
}

struct perf_counters pmcs_diff(struct perf_counters* a, struct perf_counters* b)
{
    struct perf_counters res;
    res.l1_references = a->l1_references - b->l1_references;
	res.l1_refills = a->l1_refills - b->l1_refills;
	res.l2_references = a->l2_references - b->l2_references;
	res.l2_refills = a->l2_refills - b->l2_refills;
    res.inst_retired = a->inst_retired - b->inst_retired;
    return res;
}
