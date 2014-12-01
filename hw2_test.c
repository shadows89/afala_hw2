#include "hw2_syscalls.h"
#include <sched.h>
// #include <sys/types.h>
// #include <unistd.h>

char* switch_reason[] = {"No reason","Task created","Task ended","Task yields","LSGORT became OVERDUE","Previous task waiting",
						"Change in the scheduling parameters","Task returns from waiting (higher prio)", "Timeslice ended (prev task)"};
char* policies[] = {"Real time (fifo and rr)","LSHORT","Other","Overdue LSHORT","Idle"};


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
		int trial_num = strtol(argv[i++], NULL, 0);
		int fibunchi = strtol(argv[i++], NULL, 0);
		pid = fork();
		if (pid==0) {
			usleep(40000);
			fibonaci(fibunchi);
			exit(0);
		} 
		else if(pid < 0){
			perror("error forking");
			exit(1);
		}
		struct sched_param param;
		param.sched_priority = 0;
		param.trial_num = trial_num;
		int res = sched_setscheduler(pid, SCHED_MQ, &param);
		if (res) {
			printf("error. setsched ret= %d,errno value=%d\n",res,errno);
		}
		else {
			printf("trial number=%d, fib=%d, pid=%d is_mq=%d\n", trial_num, fibunchi,pid,is_MQ(pid));
		}
	}
	while (wait() != -1);
	struct switch_info info[150];
	int len=get_scheduling_statistic(info);
	if (len < 0) {
		printf("get statistics error,ret=%d,errno=%d\n",len,errno);
		exit (-1);
	}
	printf("prev_p-next_p---prev_policy---next_policy---time---reason\n");
	for (i = 0; i < len; i++)
		printf("%04d     %04d    %8s   %8s       %d %s\n",info[i].previous_pid, info[i].next_pid,
			policies[info[i].previous_policy],policies[info[i].next_policy],
			info[i].time,switch_reason[info[i].reason]);
	return 0;
}