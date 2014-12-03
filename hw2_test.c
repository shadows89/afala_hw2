#include "hw2_syscalls.h"
//#include <sched.h>
#include <sys/types.h>
#include <unistd.h>

char* switch_reason[] = {"No reason","Task created","Task ended","Task yields","LSGORT became OVERDUE","Previous task waiting",
						"Change in the scheduling parameters","Task returns from waiting (higher prio)", "Timeslice ended (prev task)"};
char* policies[] = {"Real time (fifo and rr)","LSHORT","Other","Overdue LSHORT","Idle"};

#define LOG_ARRAY_SIZE 150								/* ADDED log array for test from here*/
#define MAX_EVENTS_TO_LOG 30

#define SCHED_OTHER		0
#define SCHED_FIFO		1
#define SCHED_RR		2
#define SCHED_LSHORT    3 /* ADDED */

/* ADDED STRUCT */
struct lshort_sched_param {
	int requested_time;
	int level;
};

struct sched_param {
	int sched_priority;
	struct lshort_sched_param lshort_params;  /* ADDED */
};

int fibonaci(int n) {
	if (n < 2) {
		return n;
	}
	return fibonaci(n-1) + fibonaci(n-2);
}

int main(int argc, char** argv){
	int pid;
	int i;
	for (i = 1; i < argc; ){
		int requested_time = strtol(argv[i++], NULL, 0);
		if(requested_time < 0){
			printf("Invalid requested time!!!\n");
			exit(0);
		}
		int level = strtol(argv[i++], NULL, 0);
		if(level < 1 || level > 50 ){
			printf("Invalid level!!!\n");
			exit(0);
		}
		int fib_num= strtol(argv[i++], NULL, 0);
		
		pid = fork();
		if (pid==0) {
			usleep(40000);
			fibonaci(fib_num);
			exit(0);
		} 
		else if(pid<0){
			perror("error forking");
			exit(1);
		}

		struct sched_param param;

		param.sched_priority = 0;
		param.lshort_params.requested_time = requested_time;
		param.lshort_params.level = level;

		int res = sched_setscheduler(pid, SCHED_LSHORT, &param);
		if (res)
			printf("error. setsched ret= %d,errno value=%d\n", res, errno);
		else
			printf("level=%d, fib_num=%d, pid=%d\n", level, fib_num, pid);
	}
	while (wait() != -1);
	struct switch_info info[LOG_ARRAY_SIZE];
	int len = get_scheduling_statistic(info);
	if (len<0) {
		printf("get statistics error, ret=%d, errno=%d\n", len, errno);
		exit (-1);
	}
	printf("prev_p-next_p---prev_policy---next_policy---time---reason\n");
	for (i = 0; i < len; i++)
		printf("%04d     %04d    %8s   %8s       %lu %s\n",
			info[i].previous_pid, info[i].next_pid,
			policies[info[i].previous_policy], policies[info[i].next_policy],
			info[i].time, switch_reason[info[i].reason]);
	return 0;
}
