#include "errno.h"

struct switch_info {
	int previous_pid;
	int next_pid;
	int previous_policy;
	int next_policy;
	unsigned long time;
	int reason;
};

static inline int get_scheduling_statistic(struct switch_info* info) {
	long __res;
	__asm__ volatile (
		"movl $244, %%eax;\n\t"
		"movl %1, %%ebx;\n\t"
		"int $0x80;\n\t"
		"movl %%eax,%0\n\t"

		: "=m" (__res)

		: "m" (info)

		: "%eax","%ebx"
	);
	if ((unsigned long)(__res) >= (unsigned long)(-125)) {
		errno = -(__res);
		__res = -1;
	}
	return (int)(__res);
}

static inline int is_MQ(int pid) {
	long __res;
	__asm__ volatile (
		"movl $243, %%eax;\n\t"
		"movl %1, %%ebx;\n\t"
		"int $0x80;\n\t"
		"movl %%eax,%0\n\t"

		: "=m" (__res)

		: "m" (pid)


		: "%eax","%ebx"
	);
	if ((unsigned long)(__res) >= (unsigned long)(-125)) {
		errno = -(__res);
		__res = -1;
	}
	return (int)(__res);
}

