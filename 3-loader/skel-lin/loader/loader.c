/*
 * Loader Implementation
 *
 * 2018, Operating Systems
 */

/* Defines to remove Visual Studio Code errors */
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

/* DIE macro from the laboratory to crash on system call fail */
#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(EXIT_FAILURE);				\
		}							\
	} while (0)

#define ALLOCATED 1

/* File parsed information */
static so_exec_t *exec;
/* Page size on the system */
static int page_size;
/* Used to send SIGSEGV */
static struct sigaction old_action;
/* Used to read from the file */
static char *file_data;
/* File descriptor */
static int fd;

/* Reads from the file in a loop to guarantee the data was read */
static int loop_read(int fd, char *buffer, unsigned int offset, int nr_to_read)
{
	int total_read = 0, step_read;

	lseek(fd, offset, SEEK_SET);
	while (total_read != nr_to_read && step_read != 0) {
		step_read = read(fd, buffer + total_read, nr_to_read);
		DIE(step_read < 0, "read failed");
		total_read += step_read;
	}
	return total_read;
}

/* SIGSEGV signal handler function (with step by step comments) */
static void segv_handler(int signumber, siginfo_t *info, void *context)
{
	static int previous_segmno = -1;
	int pageno, i, segmno, ret;
	int pages_crt, first_page, last_page;
	char *program_memory;
	int last_seg = exec->segments_no - 1;

	if (info->si_signo != SIGSEGV) {
		old_action.sa_sigaction(signumber, info, context);
		return;
	}

	/* Find the coresponding segment */
	segmno = -1;
	for (i = 0; i < last_seg; ++i) {
		if ((uintptr_t)info->si_addr >= exec->segments[i].vaddr &&
		(uintptr_t)info->si_addr < exec->segments[i + 1].vaddr) {
			segmno = i;
			break;
		}
	}

	/* Segment not found or it's the last segment */
	if (segmno == -1 &&
	(uintptr_t)info->si_addr >= exec->segments[last_seg].vaddr &&
	(uintptr_t)info->si_addr < exec->segments[last_seg].vaddr +
				exec->segments[last_seg].mem_size) {
		segmno = last_seg;
	}

	/* Segment not found - invalid access outside the program */
	if (segmno == -1) {
		old_action.sa_sigaction(signumber, info, context);
		return;
	}

	pageno = ((char *)info->si_addr - (char *)exec->base_addr) / page_size;
	first_page = (exec->segments[segmno].vaddr -
			exec->base_addr) / page_size;
	last_page = (ALIGN_UP(exec->segments[segmno].mem_size +
			exec->segments[segmno].vaddr, page_size) -
			exec->base_addr) / page_size;
	pages_crt = pageno - first_page;

	/* Check if the page was allocated before */
	if (exec->segments[segmno].data != NULL &&
		((char *)exec->segments[segmno].data)[pages_crt] == ALLOCATED) {
		old_action.sa_sigaction(signumber, info, context);
		return;
	}
	if (exec->segments[segmno].data == NULL) {
		exec->segments[segmno].data = calloc(sizeof(char),
		last_page - first_page + 1);
	}
	((char *)exec->segments[segmno].data)[pages_crt] = ALLOCATED;

	/* Read 1 segment from the file if it's a new one */
	if (previous_segmno != segmno) {
		loop_read(fd, file_data, exec->segments[segmno].offset,
			exec->segments[segmno].file_size);
	}

	/* Map the memory for the file to access */
	program_memory = mmap((char *) exec->segments[segmno].vaddr +
				pages_crt * page_size, page_size, PROT_WRITE,
				MAP_ANONYMOUS | MAP_SHARED | MAP_FIXED, -1, 0);
	DIE(program_memory == MAP_FAILED, "mmap failed");

	/* Write 0 on the allocated page */
	memset(program_memory, 0, page_size);

	/* Copy data from the file buffer to the program memory*/
	memcpy(program_memory, file_data + pages_crt * page_size,
		(exec->segments[segmno].file_size > page_size) ? page_size :
			exec->segments[segmno].file_size);

	/* Ensure that the data has been written */
	ret = msync(program_memory, page_size, MS_SYNC);
	DIE(ret < 0, "msync failed");

	/* Change to the desired permisions */
	ret = mprotect(program_memory, page_size, exec->segments[segmno].perm);
	DIE(ret < 0, "mprotect failed");

	previous_segmno = segmno;
}

/* Sets SIGSEGV as a caught signal and assigns a handler function */
int so_init_loader(void)
{
	struct sigaction action;
	int ret;

	page_size = getpagesize();
	DIE(page_size <= 0, "gerpagesize failed");
	action.sa_sigaction = segv_handler;
	ret = sigemptyset(&action.sa_mask);
	DIE(ret == -1, "sigemptyset failed");
	ret = sigaddset(&action.sa_mask, SIGSEGV);
	DIE(ret == -1, "sigaddset failed");
	action.sa_flags = SA_SIGINFO;
	ret = sigaction(SIGSEGV, &action, &old_action);
	DIE(ret == -1, "sigaction failed");

	return 0;
}

/* Prepares the file and starts the execution */
int so_execute(char *path, char *argv[])
{
	int i, max_mem_seg = 0;

	exec = so_parse_exec(path);
	if (!exec)
		return -1;
	fd = open(path, O_RDONLY);
	DIE(fd < 0, "open failed");

	for (i = 0; i < exec->segments_no; ++i) {
		if (max_mem_seg < exec->segments[i].file_size)
			max_mem_seg = exec->segments[i].file_size;
	}
	file_data = malloc(sizeof(char) * max_mem_seg);
	DIE(file_data == NULL, "malloc failed");

	so_start_exec(exec, argv);

	return 0;
}
