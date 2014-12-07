#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syscall.h>
#include <time.h>
#include <unistd.h>

struct switch_info {
	int arr[6];
};

#define rem_time(pid) syscall(243, pid)
#define overdue_time(pid) syscall(244, pid)
#define read_stats(buf) syscall(245, buf)

static int become_lshort(unsigned int req_time, unsigned int level) {
	struct {
		int req_time;
		int level;
	} p;

	p.req_time = req_time;
	p.level = level;

	return sched_setscheduler(getpid(), 3, (struct sched_param *) &p);
}

static int run_forked(int (*func)(void *), void *param) {
	int err, ret;
	int pid;
	int pipes[2];

	if (pipe(pipes)) {
		perror("pipe");
		exit(1);
	}

	if ((pid = fork()) < 0) {
		perror("fork");
		exit(1);
	}

	if (!pid) {
		ret = func(param);
		write(pipes[1], &ret, sizeof (int));
		exit(0);
	} else {
		err = waitpid(pid, NULL, 0);
		if (err < 0) {
			perror("waitpid");
			exit(1);
		}
		read(pipes[0], &ret, sizeof (int));
		close(pipes[0]);
		close(pipes[1]);
		return ret;
	}
}

//////////////

static int __req_time_zero(void *p) {
	if (become_lshort(0, 1) == -1)
		return 1;
	return 0;
}

static int test_req_time_zero() {
	return run_forked(__req_time_zero, NULL);
}

static int __req_time_too_big(void *p) {
	if (become_lshort(30 * 1000 + 1, 1) == -1)
		return 1;
	return 0;
}

static int test_req_time_too_big() {
	return run_forked(__req_time_too_big, NULL);
}

static int __level_too_small(void *p) {
	if (become_lshort(10 * 1000, 0) == -1)
		return 1;
	return 0;
}

static int test_level_too_small() {
	return run_forked(__level_too_small, NULL);
}

static int __level_too_big(void *p) {
	if (become_lshort(10 * 1000, 51) == -1)
		return 1;
	return 0;
}

static int test_level_too_big() {
	return run_forked(__level_too_big, NULL);
}

static int __setsched_success(void *p) {
	if (become_lshort(10 * 1000, 2) != 0) {
		return 0;
	}
	return 3 == sched_getscheduler(0);
}

static int test_setsched_success() {
	return run_forked(__setsched_success, NULL);
}

static int __setsched_twice(void *p) {
	if (become_lshort(10 * 1000, 1) != 0) {
		return 0;
	}
	return become_lshort(10 * 1000, 1) == -1;
}

static int test_setsched_twice() {
	return run_forked(__setsched_twice, NULL);
}

static int __setparams_after_setsched(void *p) {
	struct {
		int a, b;
	} param = {1, 2};

	if (become_lshort(10 * 1000, 1) != 0) {
		return 0;
	}
	return sched_setparam(getpid(), (struct sched_param *) &param) == -1;
}

static int test_setparam_after_setsched() {
	return run_forked(__setparams_after_setsched, NULL);
}

static int __getparam(void *p) {
	struct {
		int req, level;
	} param;

	if (become_lshort(10 * 1000, 1) != 0) {
		return 0;
	}
	if (sched_getparam(getpid(), (struct sched_param *) &param))
		return 0;
	if (param.level != 1 || param.req != 10 * 1000)
		return 0;
	return 1;
}

static int test_getparam() {
	return run_forked(__getparam, NULL);
}

static int __rem_time_not_overdue(void *p) {
	if (become_lshort(10 * 1000, 1) != 0)
		return 0;
	return rem_time(getpid()) > 9000;
}

static int test_rem_time_not_overdue() {
	return run_forked(__rem_time_not_overdue, NULL);
}

static int __overdue_time_not_overdue(void *p) {
	if (become_lshort(10 * 1000, 1) != 0)
		return 0;
	return overdue_time(getpid()) == 0;
}

static int test_overdue_time_not_overdue() {
	return run_forked(__overdue_time_not_overdue, NULL);
}

static int __rem_time_overdue(void *p) {
	int start = time(NULL);
	if (become_lshort(10, 1) != 0) {
		return 0;
	}
	while (time(NULL) - start < 2)
		;
	return rem_time(getpid()) == 0;
}

static int test_rem_time_overdue() {
	return run_forked(__rem_time_overdue, NULL);
;
}

static int __overdue_time_overdue(void *p) {
	int start = time(NULL);
	if (become_lshort(1, 1) != 0)
		return 0;
	while (time(NULL) - start < 2)
		;
	return overdue_time(getpid()) > 0;
}

static int test_overdue_time_overdue() {
	return run_forked(__overdue_time_overdue, NULL);
}

static int __lshort_preempts_other(void *p) {
	int start;
	if (become_lshort(10 * 1000, 1) != 0)
		return 0;
	start = time(NULL);
	while(time(NULL) - start < 5)
		;
	return 1;
}

static int test_lshort_preempts_other() {
	struct timeval start, end;
	int pid;
	if (gettimeofday(&start, NULL)) {
		perror("gettimeofday");
		exit(1);
	}
	pid = fork();
	if (pid < 0) {
		perror("fork");
		exit(1);
	}
	if (!pid) {
		__lshort_preempts_other(NULL);
		exit(0);
	} else {
		sleep(1);
		if (gettimeofday(&end, NULL)) {
			perror("gettimeofday");
			exit(1);
		}
		waitpid(pid, NULL, 0);
	}

	if (end.tv_sec - start.tv_sec < 5)
		return 0;
	return 1;
}

static int __other_preempts_overdue(void *p) {
	int start;
	if (become_lshort(1, 1) != 0)
		return 0;
	while (rem_time(getpid()))
		;
	start = time(NULL);
	while (time(NULL) - start < 3)
		;
	return 1;
}

static int test_other_preempts_overdue() {
	struct timeval start, end;
	int pid;
	pid = fork();
	if (pid < 0) {
		perror("fork");
		exit(1);
	}
	if (!pid) {
		__other_preempts_overdue(NULL);
		exit(0);
	} else {
		sleep(1);
		if (gettimeofday(&start, NULL)) {
			perror("gettimeofday");
			exit(1);
		}
		waitpid(pid, NULL, 0);
		if (gettimeofday(&end, NULL)) {
			perror("gettimeofday");
			exit(1);
		}
	}

	if (end.tv_sec - start.tv_sec < 1)
		return 0;
	return 1;
}

static int ____fork_child_level(void *p) {
	struct {
		int time;
		int level;
	} param;
	sched_getparam(getpid(), (struct sched_param *) &param);
	return param.level;
}

static int __fork_child_level(void *p) {
	if (become_lshort(1000, 20) != 0)
		return 0;
	return run_forked(____fork_child_level, NULL) == 20;
}

static int test_fork_child_level() {
	return run_forked(__fork_child_level, NULL);
}

static int ____fork_child_requested_time(void *p) {
	struct {
		int time;
		int level;
	} param;
	sched_getparam(getpid(), (struct sched_param *) &param);
	return param.time;
}

static int __fork_child_requested_time(void *p) {
	int rem;
	int child_req_time;
	int assumed_time;

	if (become_lshort(1000, 20) != 0)
		return 0;
	rem = rem_time(getpid());
	child_req_time = run_forked(____fork_child_requested_time, NULL);

	assumed_time = (rem * 51) / 100;
	return (assumed_time - 10) <  child_req_time &&
		(assumed_time + 10) > child_req_time;
}

static int test_fork_requested_time() {
	return run_forked(__fork_child_requested_time, NULL);
}

static int __fork_parent_remaining_time(void *p) {
	int rem;
	int rem_after_fork;
	int assumed_time;
	if (become_lshort(1000, 20) != 0)
		return 0;
	rem = rem_time(getpid());
	switch (fork()) {
	case -1:
		perror("fork");
	case 0:
		exit(1);
	}
	rem_after_fork = rem_time(getpid());

	assumed_time = rem - (rem * 51) / 100;

	return (assumed_time - 10) < rem_after_fork &&
		(assumed_time + 10) > rem_after_fork;
}

static int test_fork_parent_remaining_time() {
	return run_forked(__fork_parent_remaining_time, NULL);
}

static int __fork_child_overdue_requested_time(void *p) {
	int child_req_time;
	if (become_lshort(1, 20) != 0)
		return 0;
	while (rem_time(getpid()))
		;
	child_req_time = run_forked(____fork_child_requested_time, NULL);

	return child_req_time == 0;
}

static int test_fork_overdue_requested_time() {
	return run_forked(__fork_child_overdue_requested_time, NULL);
}
#define NR_KIDS 4
#define FIXTURE "*3210"
static int __lshort_correct_prio(void *p) {
	pid_t children[NR_KIDS];
	int pipes[2];
	char buf[NR_KIDS + 1];
	int i;
	fd_set fds;
	FD_ZERO(&fds);

	if (pipe(pipes)) {
		perror("pipe");
		return 0;
	}
	FD_SET(pipes[0], &fds);

	if (become_lshort(30 * 1000, 50) != 0)
		return 0;

	for (i = 0; i < NR_KIDS; i++) {
		children[i] = fork();
		if (!children[i]) {
			char c = '0' + i;
			select(pipes[0] + 1, &fds, NULL, NULL, NULL);
			while (write(pipes[1], &c, 1) != 1)
				;
			exit(0);
		}
	}

	sleep(1);
	write(pipes[1], "*", 1);

	for (i = 0; i < NR_KIDS; i++) {
		waitpid(children[i], NULL, 0);
	}

	if (read(pipes[0], buf, NR_KIDS + 1) != NR_KIDS + 1)
		return 0;
	close(pipes[0]);
	close(pipes[1]);
	return memcmp(buf, FIXTURE, NR_KIDS + 1) == 0;
}

static int test_lshort_correct_prio() {
	return run_forked(__lshort_correct_prio, NULL);
}
/*
static int __lshort_correct_prio1(void *p) {
	pid_t children[3];
	int pipes[2];
	char buf[4];
	int i;
	fd_set fds;
	FD_ZERO(&fds);

	if (pipe(pipes)) {
		perror("pipe");
		return 0;
	}
	FD_SET(pipes[0], &fds);

	for (i = 0; i < 3; i++) {
		children[i] = fork();
		if (!children[i]) {
			char c = '0' + i;
			if (become_lshort(30 * 1000, 1 + i * 10) != 0)
				exit(1);

			select(pipes[0] + 1, &fds, NULL, NULL, NULL);
			while (write(pipes[1], &c, 1) != 1)
				;
			exit(0);
		}
	}

	sleep(1);
	write(pipes[1], "*", 1);

	for (i = 0; i < 3; i++) {
		waitpid(children[i], NULL, 0);
	}

	if (read(pipes[0], buf, 4) != 4)
		return 0;
	close(pipes[0]);
	close(pipes[1]);
	return memcmp(buf, "*012", 4) == 0;
}

static int test_lshort_correct_prio1() {
	return run_forked(__lshort_correct_prio1, NULL);
}

static int __lshort_correct_prio2(void *p) {
	pid_t children[3];
	int pipes[2];
	char buf[4];
	int i;
	fd_set fds;
	FD_ZERO(&fds);

	if (pipe(pipes)) {
		perror("pipe");
		return 0;
	}
	FD_SET(pipes[0], &fds);

	for (i = 0; i < 3; i++) {
		children[i] = fork();
		if (!children[i]) {
			char c = '0' + i;
			if (become_lshort(3 * 1000, 1 + i * 10) != 0)
				exit(1);
			select(pipes[0] + 1, &fds, NULL, NULL, NULL);
			while (write(pipes[1], &c, 1) != 1)
				;
			exit(0);
		}
	}
	sleep(1);
	write(pipes[1], "*", 1);

	for (i = 0; i < 3; i++) {
		waitpid(children[i], NULL, 0);
	}

	if (read(pipes[0], buf, 4) != 4)
		return 0;
	close(pipes[0]);
	close(pipes[1]);
	return memcmp(buf, "*210", 4) == 0;
}

static int test_lshort_correct_prio2() {
	return run_forked(__lshort_correct_prio2, NULL);
}*/

static int test_read_stats_150_records() {
	struct switch_info infos[150];
	return 150 == read_stats(&infos);
}

///////////////

struct test_def {
	int (*func)();
	const char *name;
};

#define DEFINE_TEST(func) { func, #func }

struct test_def tests[] = {
	DEFINE_TEST(test_req_time_zero),
	DEFINE_TEST(test_req_time_too_big),
	DEFINE_TEST(test_level_too_small),
	DEFINE_TEST(test_level_too_big),
	DEFINE_TEST(test_setsched_success),
	DEFINE_TEST(test_setsched_twice),
	DEFINE_TEST(test_setparam_after_setsched),
	DEFINE_TEST(test_getparam),
	DEFINE_TEST(test_rem_time_not_overdue),
	DEFINE_TEST(test_overdue_time_not_overdue),
	DEFINE_TEST(test_rem_time_overdue),
	DEFINE_TEST(test_overdue_time_overdue),
	DEFINE_TEST(test_lshort_preempts_other),
	DEFINE_TEST(test_other_preempts_overdue),
	DEFINE_TEST(test_fork_child_level),
	DEFINE_TEST(test_fork_requested_time),
	DEFINE_TEST(test_fork_parent_remaining_time),
	DEFINE_TEST(test_fork_overdue_requested_time),
	DEFINE_TEST(test_lshort_correct_prio),
	/*DEFINE_TEST(test_lshort_correct_prio1),
	DEFINE_TEST(test_lshort_correct_prio2),*/
	DEFINE_TEST(test_read_stats_150_records),
	{ NULL, "The end" },
};

int main() {
	struct test_def *current = &tests[0];
	while (current->func) {
		printf("%-35s:\t%s\n",
		       current->name,
		       (1 == current->func()) ? "PASS" : "FAIL");
		current++;
	};
	return 0;
}
