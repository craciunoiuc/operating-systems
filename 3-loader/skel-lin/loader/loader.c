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
static char *file_data;
static int fd;
static const char allocated = 1;

static void loop_read(int fd, char *buffer, unsigned int offset, int nr_to_read) {
	int real_read = 0;
	lseek(fd, offset, SEEK_SET);

	while (real_read != nr_to_read) {
		real_read += read(fd, buffer + real_read, nr_to_read);
	}
}

static void segv_handler(int signum, siginfo_t *info, void *context) {
	int pageno, i, segmno;
	int pages_crt, first_page, last_page;
	if (info->si_signo != SIGSEGV) {
		old_action.sa_sigaction(signum, info, context);
		return;
	}


	// TODO INLOCUIT 0 LA SEGMENTE (Aliniat in ambele directii?)

	// Find coresponding segment
	segmno = -1;
	//printf("%p == %p == %p\n", (uintptr_t )info->si_addr,
	//	exec->segments[0].vaddr, exec->segments[1].vaddr);
	for (i = 0; i < exec->segments_no - 1; ++i) {
		if ((uintptr_t )info->si_addr >= exec->segments[i].vaddr &&
		(uintptr_t)info->si_addr < exec->segments[i + 1].vaddr) {
			segmno = i;
			break;
		}
	}
	
	// Segment not found or it's the last segment
	if (segmno == -1 &&
	(uintptr_t)info->si_addr >= exec->segments[exec->segments_no - 1].vaddr &&
	(uintptr_t)info->si_addr < exec->segments[exec->segments_no - 1].vaddr +
			exec->segments[exec->segments_no - 1].mem_size) {
		segmno = exec->segments_no - 1;
	}
	// Segment not found - invalid access outside program
	if (segmno == -1) {
		old_action.sa_sigaction(signum, info, context);
		return;
	}
	//puts("SEGMENT FOUND");
	// Segment already mapped - invalid access outside segment
	// pageno <- a cata pagina din program
	pageno = ((char *)info->si_addr - (char *)exec->base_addr) / pageSize;
	first_page = (exec->segments[segmno].vaddr - exec->base_addr) / pageSize;
	last_page = (ALIGN_UP(exec->segments[segmno].mem_size +
			exec->segments[segmno].vaddr, pageSize) -
			exec->base_addr) / pageSize;
	pages_crt = pageno - first_page;
	//printf("pageno=%d first=%d last=%d crt=%d\n", pageno, first_page, last_page, pages_crt);
	if (exec->segments[segmno].data != NULL &&
		((char *)exec->segments[segmno].data)[pages_crt] == allocated) {
		old_action.sa_sigaction(signum, info, context);
		return;
	} else {
		//puts("Not allocated");
		if (exec->segments[segmno].data == NULL) {
			//puts("First time in this segment");
			exec->segments[segmno].data = calloc(sizeof(char),
			last_page - first_page + 1);
		}
		//puts("Marking as allocated");
		((char *)exec->segments[segmno].data)[pages_crt] = allocated;
	}
	//puts("ALO");

	//printf("maxno=%d pageno=%d mem=%p prg=%p diff=%d\n", exec->segments_no,
	//	pageno, info->si_addr, program_memory,
	//	((char *)info->si_addr) - program_memory);


	// Read from file - 1 segment
	loop_read(fd, file_data, exec->segments[segmno].offset,
		exec->segments[segmno].file_size);
	
	// Map the memory for the file to access
	program_memory = mmap((char *) exec->segments[segmno].vaddr + pages_crt * pageSize,
				pageSize,
				PROT_WRITE,
				MAP_ANONYMOUS | MAP_SHARED | MAP_FIXED, -1, 0);
	//puts("ALO");

	// Write 0 on the pages
	memset(program_memory, 0, pageSize);
	
	// Copy the data from the file - TODO
	memcpy(program_memory, file_data + pages_crt * pageSize,
		(exec->segments[segmno].file_size > pageSize) ? pageSize : exec->segments[segmno].file_size);
	
	// Ensure that the data has been written
	msync(program_memory, pageSize, MS_SYNC);
	
	// Change to the desired permisions
	mprotect(program_memory, pageSize, exec->segments[segmno].perm);

	//old_action.sa_sigaction(signum, info, context);
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
	int i, max_mem_seg = 0;

	exec = so_parse_exec(path);
	if (!exec) {
		return -1;
	}
	fd = open(path, O_RDONLY);
	DIE(fd < 0, "open");
	for (i = 0; i < exec->segments_no; ++i) {
		if (max_mem_seg < exec->segments[i].file_size) {
			max_mem_seg = exec->segments[i].file_size;
		}
	}
	file_data = malloc(sizeof(char) * max_mem_seg);
	DIE(file_data == NULL, "malloc");

	so_start_exec(exec, argv);

	return 0;
}
