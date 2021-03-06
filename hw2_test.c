#include "hw2_syscalls.h"
//#include <sched.h>
#include <sys/types.h>
#include <unistd.h>

char* switch_reason[] = {"No reason", "Task created", "Task ended", "Task yields", "LSHORT became OVERDUE", "Previous task waiting",
						"Change in the scheduling parameters", "Task returns from waiting (higher prio)", "Timeslice ended (prev task)"};
char* policies[] = {"Other", "FIFO", "RR", "LSHORT"};

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
	union{
		int sched_priority;
		struct lshort_sched_param lshort_params;  /* ADDED */
	};
};

int fibonaci(int n) {
	if (n < 2) {
		return n;
	}
	return fibonaci(n-1) + fibonaci(n-2);
}

int main(int argc, char** argv){
	int pid, i;
	for (i = 1; i < argc; ){

		int requested_time = atoi(argv[i++]);
		if(requested_time < 0 || requested_time > 30000){ // ADDED for checker
			printf("Invalid requested time!!!\n");
			exit(1);
		}

		int level = atoi(argv[i++]);
		if(level < 1 || level > 50 ){
			printf("Level ERROR!\n");
			exit(2);
		}
	
		int fib_num= atoi(argv[i++]);
		
		pid = fork();
		
		if  (pid < 0) {
			perror("Fork ERROR!\n");
			exit(3);
		}
		else if (!pid) {
			usleep(60000);
			fibonaci(fib_num);
			exit(0);
		} 

		struct sched_param param;

		param.lshort_params.requested_time = requested_time;
		param.lshort_params.level = level;

		int res = sched_setscheduler(pid, SCHED_LSHORT, &param);
	
		if (res)
			printf("sched_setscheduler ERROR: ret= %d, errno=%d pid=%d level=%d requested_time=%d, fib_num=%d\n", res, errno, pid, level, requested_time, fib_num);
		else
			printf("requested_time=%d level=%d, fib_num=%d, pid=%d\n", requested_time,level, fib_num, pid);
	}
	while (wait() != -1);
	struct switch_info info[LOG_ARRAY_SIZE];
	int logs_number = get_scheduling_statistic(info);
	if (logs_number < 0) {
		printf("get_scheduling_statistic ERROR: ret=%d, errno=%d\n", logs_number, errno);
		exit (-1);
	}
	printf("prev_pid   next_pid       prev_policy      next_policy     time    reason\n");
	for (i = 0; i < logs_number; i++)
		printf("%04d       %04d    %14s   %14s         %lu %s\n",
			info[i].previous_pid, info[i].next_pid,
			policies[info[i].previous_policy], policies[info[i].next_policy],
			info[i].time, switch_reason[info[i].reason]);
	return 0;
}
