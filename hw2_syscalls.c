	#include <../include/linux/errno.h>
	#include <../include/linux/sched.h>
	#include <../include/linux/kernel.h>

struct task_struct* checkPid(int pid){
	struct task_struct* pd = NULL;
	if(pid == 0){
		pd = &init_task;
		return pd->p_pptr;
	} else	
		return find_task_by_pid(pid);
}


int sys_lshort_query_remaining_time(int pid){
	if(pid < 0)
		return -EINVAL;
	struct task_struct *pd = checkPid(pid);
	if(pd == NULL || pd->policy != SCHED_LSHORT)
		return -EINVAL;

}

int sys_lshort_query_overdue_time(int pid){
	if(pid < 0)
		return -EINVAL;
	struct task_struct *pd = checkPid(pid);
	if(pd == NULL || pd->policy != SCHED_LSHORT)
		return -EINVAL;
}