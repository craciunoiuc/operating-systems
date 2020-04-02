/*
 * Loader Implementation
 *
 * 2018, Operating Systems
 */
#define _XOPEN_SOURCE 700
#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include "exec_parser.h"

#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(EXIT_FAILURE);				\
		}							\
	} while (0)

static so_exec_t *exec;
static int pageSize;
static struct sigaction old_action;
static char *program_memory;
static char *file_reading;

static void segv_handler(int signum, siginfo_t *info, void *context) {
	int pageno;

	if (info->si_signo != SIGSEGV) {
		old_action.sa_sigaction(signum, info, context);
		return;
	}
	/* info->si_addr <- adresa care a dat fault*/
	pageno = ((char*)info->si_addr - program_memory) / pageSize;
	if (pageno > exec->segments_no || pageno < 0) {
		old_action.sa_sigaction(signum, info, context);
		return;
	}

	/* old_action.sa_sigaction(signum, info, context); */
}

static void set_signal(void) {
	struct sigaction action;
	int ret;

	action.sa_sigaction = segv_handler;
	sigemptyset(&action.sa_mask);
	sigaddset(&action.sa_mask, SIGSEGV);
	action.sa_flags = SA_SIGINFO;

	ret = sigaction(SIGSEGV, &action, &old_action);
	DIE(ret == -1, "sigaction");
}

int so_init_loader(void) {
	pageSize = getpagesize();
	set_signal();
	return 0;
}

int so_execute(char *path, char *argv[]) {
	int fd = -1;

	exec = so_parse_exec(path);
	if (!exec) {
		return -1;
	}
	fd = open(path, O_RDONLY);
	DIE(fd < 0, "open");
	/*file_reading = mmap(NULL, exec->segments_no * pageSize,
				PROT_READ, MAP_SHARED, fd, 0);
	*/
	program_memory = mmap(NULL, exec->segments_no * pageSize,
		PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	exec->base_addr = (uintptr_t) program_memory;
	so_start_exec(exec, argv);

	return 0;
}
