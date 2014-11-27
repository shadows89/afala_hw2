#ifndef __LINUX_HW2_SYSCALLS_H
#define __LINUX_HW2_SYSCALLS_H

#include <errno.h>
#include <sched.h>

int lshort_query_remaining_time(int pid){
	int res;
	__asm__ 
	(
 		"int $0x80;"
 		: "=a" (res) 
 		: "0" (243) ,"b" (pid)
 		: "memory"
	);
	if(res == -EINVAL){
		errno = EINVAL;
		return -1;
	}
	if(res == -ESRCH){
		errno = ESRCH;
		return -1;
	}
	return (int) res;
}

int lshort_query_overdue_time(int pid){
	int res;
	__asm__ 
	(
 		"int $0x80;"
 		: "=a" (res) 
 		: "0" (244) ,"b" (pid)
 		: "memory"
	);
	if(res == -EINVAL){
		errno = EINVAL;
		return -1;
	}
	if(res == -ESRCH){
		errno = ESRCH;
		return -1;
	}
	return (int) res;
}

int get_scheduling_statistic(struct switch_info *){  /* Test */
	int res;
	__asm__ 
	(
 		"int $0x80;"
 		: "=a" (res) 
 		: "0" (245) ,"b" (switch_info *)
 		: "memory"
	);
	if(res == -1){
		errno = -1;
		return -1;
	}
	return (int) res;
}

#endif
