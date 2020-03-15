#include "./so_internal_stdio.h"

/* Initialises the SO_FILE structure and opens the file in the required mode */
SO_FILE *so_fopen(const char *pathname, const char *mode)
{
	SO_FILE *file_ptr = malloc(sizeof(SO_FILE));

	if (file_ptr == NULL)
		exit(12);
	file_ptr->fd       = 0;
	file_ptr->buf_size = 0;
	file_ptr->flags    = 0;
	file_ptr->wait     = 0;
	file_ptr->buf_data = 0;
	file_ptr->error    = 0;
	file_ptr->file_pos = 0;
	file_ptr->curr_ptr = NULL;
	file_ptr->buffer   = NULL;
	file_ptr->last_op  = NO_OP;

	switch (mode[0]) {
		case 'r': {
			if (mode[1] == '+') {
				file_ptr->fd = open(pathname, O_RDWR);
				file_ptr->flags |= O_RDWR;
			} else {
				file_ptr->fd = open(pathname, O_RDONLY);
				file_ptr->flags |= O_RDONLY;
			}
			break;
		}

		case 'w': {
			if (mode[1] == '+') {
				file_ptr->flags = O_RDWR | O_CREAT | O_TRUNC;
				file_ptr->fd = open(pathname,
						file_ptr->flags, 0777);
			} else {
				file_ptr->flags = O_WRONLY | O_CREAT | O_TRUNC;
				file_ptr->fd = open(pathname,
						file_ptr->flags, 0777);
			}
			break;
		}

		case 'a': {
			if (mode[1] == '+') {
				file_ptr->flags = O_RDWR | O_APPEND | O_CREAT;
				file_ptr->fd = open(pathname,
						file_ptr->flags, 0777);

			} else {
				file_ptr->flags = O_WRONLY | O_CREAT | O_APPEND;
				file_ptr->fd = open(pathname,
						file_ptr->flags, 0777);
			}
			break;
		}

		default: {
			free(file_ptr);
			return NULL;
		}
	}
	if (file_ptr->fd <= 0) {
		free(file_ptr);
		return NULL;
	}
	file_ptr->buf_size = 4096;
	file_ptr->buffer = malloc(sizeof(char) * file_ptr->buf_size);
	file_ptr->curr_ptr = file_ptr->buffer;
	if (file_ptr->buffer == NULL)
		exit(12);
	return file_ptr;
}

/* Closes the file and frees data from the stream after flushing it */
int so_fclose(SO_FILE *stream)
{
	int ret = 0;
	char fail = 0;

	if (stream) {
		if (stream->buffer && stream->fd &&
			stream->last_op == WRITE_OP) {
			ret = so_fflush(stream);
			if (ret < 0) {
				stream->error = SO_EOF;
				fail = 1;
			}
		}
		ret = close(stream->fd);
		if (stream->buffer)
			free(stream->buffer);
		free(stream);
		return fail ? SO_EOF : ret;
	}
	return SO_EOF;
}

/* Returns the file descriptor in the stream */
int so_fileno(SO_FILE *stream)
{
	return stream ? stream->fd : SO_EOF;
}

/* Buffers data from the file into the stream and returns one character */
int so_fgetc(SO_FILE *stream)
{
	int ret = 0;

	if (stream) {
		stream->last_op = READ_OP;
		stream->file_pos++;
		if (stream->curr_ptr - stream->buffer >= stream->buf_size) {
			stream->curr_ptr = stream->buffer;
			stream->buf_data = 0;
		}
		if (stream->curr_ptr - stream->buffer >= stream->buf_data) {
			ret = read(stream->fd, stream->curr_ptr,
				stream->buf_size - stream->buf_data);
			if (ret == 0) {
				stream->error = SO_EOF;
				return SO_EOF;
			}
			stream->buf_data += ret;
		}
		stream->curr_ptr++;
		return *(stream->curr_ptr - 1);
	}
	return SO_EOF;
}

/* Copies chunks of memory from the buffer into ptr, and, if needed, gets
 * more information from the file into the buffer.
 */
size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	int ret = 0;
	size_t bytes = size * nmemb;
	size_t read_nmemb = 0;

	if (stream) {
		stream->last_op = READ_OP;
		stream->file_pos += bytes;
		if (stream->buf_data == 0) {
			stream->curr_ptr = stream->buffer;
		} else {
			if (stream->buf_data > bytes) {
				memcpy(ptr, stream->curr_ptr, bytes);
				stream->curr_ptr += bytes;
				stream->buf_data -= bytes;
				return nmemb;
			}
			memcpy(ptr, stream->curr_ptr, stream->buf_data);
			ptr += stream->buf_data;
			nmemb -= stream->buf_data / size;
			bytes -= stream->buf_data;
			stream->curr_ptr = stream->buffer;
			read_nmemb += stream->buf_data / size;
			stream->buf_data = 0;
		}
		while (bytes > 0) {
			ret = read(stream->fd, stream->curr_ptr,
				stream->buf_size - stream->buf_data);
			if (ret == 0) {
				stream->error = SO_EOF;
				return read_nmemb;
			}
			if (ret < 0) {
				stream->error = SO_EOF;
				return 0;
			}
			stream->buf_data += ret;
			if (bytes > stream->buf_data) {
				memcpy(ptr, stream->curr_ptr, stream->buf_data);
				ptr += stream->buf_data;
				bytes -= stream->buf_data;
				stream->curr_ptr = stream->buffer;
				read_nmemb += stream->buf_data / size;
				stream->buf_data = 0;
			} else {
				memcpy(ptr, stream->curr_ptr, bytes);
				ptr += bytes;
				stream->curr_ptr += bytes;
				stream->buf_data -= bytes;
				read_nmemb += bytes / size;
				bytes = 0;
			}
		}
		return read_nmemb;
	}
	return SO_EOF;
}

/* Copies on character into the buffer and writes it if \n was found or
 * the buffer is full.
 */
int so_fputc(int c, SO_FILE *stream)
{
	char buffer = c;
	int  ret = 0;

	if (stream) {
		stream->last_op = WRITE_OP;
		stream->file_pos++;
		*(stream->curr_ptr) = buffer;
		stream->curr_ptr++;
		stream->buf_data++;
		if (buffer == '\n' || stream->buf_data == stream->buf_size) {
			ret = write(stream->fd, stream->buffer,
				stream->buf_data);
			if (ret <= 0) {
				stream->error = SO_EOF;
				return SO_EOF;
			}
			stream->curr_ptr -= ret;
			stream->buf_data -= ret;
		}
		return c;
	}
	return SO_EOF;
}

/* Writes chunks of data in the buffer and copies it into the file if
 * the buffer filled up
 */
size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	ssize_t space_left = 0, write_nmemb = 0, ret;
	ssize_t bytes = size * nmemb;

	if (stream) {
		stream->last_op = WRITE_OP;
		stream->file_pos += bytes;
		if (stream->buf_data == 0)
			stream->curr_ptr = stream->buffer;

		/* There is space in the buffer */
		space_left = stream->buf_size - stream->buf_data;
		if (space_left >= bytes) {
			memcpy(stream->curr_ptr, ptr, bytes);
			stream->curr_ptr += bytes;
			stream->buf_data += bytes;
			return nmemb;
		}
		memcpy(stream->curr_ptr, ptr, space_left);
		stream->curr_ptr += space_left;
		bytes -= space_left;
		ptr += space_left;
		write_nmemb += space_left / size;
		stream->buf_data += space_left;

		while (bytes > 0) {
			ret = write(stream->fd,
				stream->buffer, stream->buf_data);
			if (ret <= 0) {
				stream->error = SO_EOF;
				return 0;
			}
			if (ret < stream->buf_data) {
				ret = loop_write(stream->fd,
						stream->buffer + ret,
						stream->buf_data - ret);
				stream->buf_data = 0;
				stream->curr_ptr = stream->buffer;
				stream->file_pos += ret;
			} else {
				stream->buf_data -= ret;
				stream->curr_ptr -= ret;
			}
			space_left = stream->buf_size - stream->buf_data;
			if (space_left >= bytes) {
				memcpy(stream->curr_ptr, ptr, bytes);
				stream->curr_ptr += bytes;
				stream->buf_data += bytes;
				write_nmemb += bytes / size;
				bytes = 0;
			} else {
				memcpy(stream->curr_ptr, ptr, space_left);
				stream->curr_ptr += space_left;
				stream->buf_data += space_left;
				ptr += space_left;
				bytes -= space_left;
				write_nmemb += space_left / size;
			}
		}
		return write_nmemb;
	}
	return SO_EOF;
}

/* Writes all data from the buffer into the file */
int so_fflush(SO_FILE *stream)
{
	ssize_t ret;

	if (stream) {
		ret = write(stream->fd, stream->buffer, stream->buf_data);
		if (ret <= 0) {
			stream->error = SO_EOF;
			return ret;
		}
		stream->file_pos += ret;
		if (ret < stream->buf_data) {
			ret = loop_write(stream->fd, stream->buffer + ret,
					stream->buf_data - ret);
			stream->buf_data = 0;
			stream->curr_ptr = stream->buffer;
			stream->file_pos += ret;
		} else {
			stream->curr_ptr -= ret;
			stream->buf_data -= ret;
		}
		return 0;
	}
	return SO_EOF;
}

/* Clears the buffer and moves the cursor to the wanted position*/
int so_fseek(SO_FILE *stream, long offset, int whence)
{
	long ret = 0;

	if (stream) {
		if (whence != SEEK_CUR && whence != SEEK_END &&
			whence != SEEK_SET)
			return -1;
		if (stream->last_op == READ_OP) {
			stream->buf_data = 0;
			stream->curr_ptr = stream->buffer;
		}
		if (stream->last_op == WRITE_OP)
			ret = so_fflush(stream);
		if (ret < 0)
			return -1;
		stream->last_op = NO_OP;
		ret = lseek(stream->fd, offset, whence);
		if (ret < 0)
			return -1;
		stream->file_pos = ret;
		return 0;
	}
	return -1;
}

/* Returns the position in the file */
long so_ftell(SO_FILE *stream)
{
	if (stream)
		return stream->file_pos;
	return -1;
}

/* Checks if the current position is equal to the last position in the file */
int so_feof(SO_FILE *stream)
{
	ssize_t size, curr_size, ret;

	if (stream) {
		curr_size = lseek(stream->fd, 0, SEEK_CUR);
		size = lseek(stream->fd, 0, SEEK_END);
		if (curr_size < 0 || size < 0 || size < stream->file_pos)
			return SO_EOF;
		ret = lseek(stream->fd, curr_size, SEEK_SET);
		if (ret < 0)
			return SO_EOF;
		return 0;
	}
	return SO_EOF;
}

/* Returns an error if an error has happened sometime in execution */
int so_ferror(SO_FILE *stream)
{
	return stream ? stream->error : SO_EOF;
}

/* Creates a pipe and forks the process executing a shell script to read from */
SO_FILE *read_pipe(const int pipe_read, const int pipe_write,
		const char *shell, int *ret, pid_t *pid, int *fds,
		const char **argvs, SO_FILE *stream)
{
	*ret = pipe(fds);
	if (*ret < 0)
		return NULL;
	*pid = fork();
	switch (*pid) {
		case -1: {
			/* Error */
			close(fds[pipe_read]);
			close(fds[pipe_write]);
			return NULL;
		}

		case 0: {
			/* Child */
			*ret = close(fds[pipe_read]);
			if (*ret < 0)
				return NULL;

			*ret = dup2(fds[pipe_write], STDOUT_FILENO);
			if (*ret < 0)
				return NULL;

			*ret = close(fds[pipe_write]);
			if (*ret < 0)
				return NULL;

			execvp(shell, (char * const *) argvs);
			exit(-1);
		}

		default: {
			/* Parent */
			*ret = close(fds[pipe_write]);
			if (*ret < 0)
				return NULL;

			stream = malloc(sizeof(SO_FILE));
			if (!stream)
				exit(12);
			stream->fd       = fds[pipe_read];
			stream->buf_size = 4096;
			stream->flags    = 0;
			stream->wait     = *pid;
			stream->buf_data = 0;
			stream->error    = 0;
			stream->file_pos = 0;
			stream->curr_ptr = NULL;
			stream->buffer   = NULL;
			stream->last_op  = NO_OP;
			stream->buffer = malloc(sizeof(char) *
						stream->buf_size);
			stream->curr_ptr = stream->buffer;
			if (!stream->buffer)
				exit(12);
			return stream;
		}
	}
}

/* Creates a pipe and forks the process executing a shell script to write to */
SO_FILE *write_pipe(const int pipe_read, const int pipe_write,
		const char *shell, int *ret, pid_t *pid, int *fds,
		const char **argvs, SO_FILE *stream)
{
	*ret = pipe(fds);
	if (*ret < 0)
		return NULL;
	*pid = fork();
	switch (*pid) {
		case -1: {
			/* Error */
			close(fds[pipe_write]);
			close(fds[pipe_read]);
			return NULL;
		}

		case 0: {
			/* Child */
			*ret = close(fds[pipe_write]);
			if (*ret < 0)
				return NULL;

			*ret = dup2(fds[pipe_read], STDIN_FILENO);
			if (*ret >= 0)
				return NULL;

			execvp(shell, (char * const *)argvs);
			exit(-1);
		}

		default: {
			/* Parent */
			*ret = close(fds[pipe_read]);
			if (*ret < 0)
				return NULL;
			stream = malloc(sizeof(SO_FILE));
			if (!stream)
				exit(12);
			stream->fd       = fds[pipe_write];
			stream->buf_size = 4096;
			stream->flags    = 0;
			stream->wait     = *pid;
			stream->buf_data = 0;
			stream->error    = 0;
			stream->file_pos = 0;
			stream->curr_ptr = NULL;
			stream->buffer   = NULL;
			stream->last_op  = NO_OP;
			stream->buffer = malloc(sizeof(char) *
					stream->buf_size);
			stream->curr_ptr = stream->buffer;
			if (!stream->buffer)
				exit(12);
			return stream;
		}
	}
}

/* Combines popen for both reading and writing (not at the same time) */
SO_FILE *so_popen(const char *command, const char *type)
{
	pid_t pid;
	int fds[2], ret;
	const int pipe_read = 0;
	const int pipe_write = 1;
	const char *argvs[4] = {"sh", "-c", command, NULL};
	const char *shell = "sh";
	SO_FILE *stream = NULL;

	if (!command || !type)
		return NULL;
	if (type[0] == 'r') {
		return read_pipe(pipe_read, pipe_write, shell,
				&ret, &pid, fds, argvs, stream);
	}
	if (type[0] == 'w') {
		return write_pipe(pipe_read, pipe_write, shell,
				&ret, &pid, fds, argvs, stream);
	}
	return NULL;
}

/* Waits for the child process to finish and closes the stream */
int so_pclose(SO_FILE *stream)
{
	int ret, status, close_ret;

	if (stream && stream->wait > 0) {
		ret = waitpid(stream->wait, &status, 0);
		close_ret = close(stream->fd);
		if (stream->buffer)
			free(stream->buffer);
		free(stream);
		if (ret < 0 || close_ret < 0)
			return -1;
		return ret;
	}
	return -1;
}

/* Small loop write implementation to guarantee data has been written */
size_t loop_write(int fd, void *buf, size_t count)
{
	ssize_t ret = 1, totalWritten = 0;

	while (ret > 0 && count > 0) {
		ret = write(fd, buf, count);
		if (ret < 0)
			return 0;
		buf += ret;
		count -= ret;
		totalWritten += ret;
	}
	return (ret == 0) ? -totalWritten : totalWritten;
}
