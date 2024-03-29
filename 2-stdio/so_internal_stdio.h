#ifndef SO_INTERNAL_STDIO_H
#define SO_INTERNAL_STDIO_H

#include "./util/so_stdio.h"


#if defined(__linux__)
#define FUNC_DECL_PREFIX
#include <fcntl.h>    /* Open flags */
#include <unistd.h>   /* File I/O */
#include <sys/wait.h> /* waitpid */
#elif defined(_WIN32)
#include <Windows.h>

#ifdef DLL_EXPORTS
#define FUNC_DECL_PREFIX __declspec(dllexport)
#else
#define FUNC_DECL_PREFIX __declspec(dllimport)
#endif

#else
#error "Unknown platform"
#endif

#include <string.h> /* memcpy */

#define NO_OP    0
#define WRITE_OP 1
#define READ_OP  2

struct _so_file {
	unsigned short buf_size;
	unsigned short buf_data;
	unsigned short wait;
	unsigned char  error;
	unsigned char  last_op;
	unsigned int   file_pos;
	unsigned int   flags;
	unsigned char *curr_ptr;
	unsigned char *buffer;
	int fd;
};

/* Creates a pipe and forks the process executing a shell script to read from */
FUNC_DECL_PREFIX SO_FILE *read_pipe(const int pipe_read, const int pipe_write,
			const char *shell, int *ret, pid_t *pid, int *fds,
			const char **argvs, SO_FILE *stream);

/* Creates a pipe and forks the process executing a shell script to write to */
FUNC_DECL_PREFIX SO_FILE *write_pipe(const int pipe_read, const int pipe_write,
			const char *shell, int *ret, pid_t *pid, int *fds,
			const char **argvs, SO_FILE *stream);

/* Utility function to guarantee count was written in buf from fd */
FUNC_DECL_PREFIX size_t loop_write(int fd, void *buf, size_t count);
#endif /* SO_INTERNAL_STDIO_H */

