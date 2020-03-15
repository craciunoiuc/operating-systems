#ifndef SO_INTERNAL_STDIO_H
#define SO_INTERNAL_STDIO_H

#include "./util/so_stdio.h"


#if defined(__linux__)
#define FUNC_DECL_PREFIX
#include <fcntl.h> /* Open flags */
#include <unistd.h> /* File I/O */
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
	unsigned char  wait;
	unsigned char  error;
	unsigned int   file_pos;
	unsigned int   flags;
	unsigned short buf_data;
	unsigned char  last_op;
	unsigned char *curr_ptr;
	unsigned char *buffer;
	int fd;
};

/* Utility function to guarantee count was read in buf from fd */
FUNC_DECL_PREFIX size_t loop_read(int fd, void *buf, size_t count);

/* Utility function to guarantee count was written in buf from fd */
FUNC_DECL_PREFIX size_t loop_write(int fd, void *buf, size_t count);
#endif /* SO_INTERNAL_STDIO_H */

