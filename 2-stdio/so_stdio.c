#include "./so_internal_stdio.h"

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
	
	switch(mode[0]) {
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
		if (stream->buffer) {
			free(stream->buffer);
		}
		free(stream);
		return fail ? SO_EOF : ret;
	}
	return SO_EOF;
}

int so_fileno(SO_FILE *stream)
{
	return stream ? stream->fd : SO_EOF;
}

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
			} else {
				memcpy(ptr, stream->curr_ptr, stream->buf_data);
				ptr += stream->buf_data;
				nmemb -= stream->buf_data / size;
				bytes -= stream->buf_data;
				stream->curr_ptr = stream->buffer;
				read_nmemb += stream->buf_data / size;
				stream->buf_data = 0;
			}
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
			ret = write(stream->fd, stream->buffer, stream->buf_data);
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

size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	ssize_t space_left = 0, write_nmemb = 0, ret;
	ssize_t bytes = size * nmemb;

	if (stream) {
		stream->last_op = WRITE_OP;
		stream->file_pos += bytes;
		if (stream->buf_data == 0) {
			stream->curr_ptr = stream->buffer;
		}
		/* There is space in the buffer */
		space_left = stream->buf_size - stream->buf_data;
		if (space_left >= bytes) {
			memcpy(stream->curr_ptr, ptr, bytes);
			stream->curr_ptr += bytes;
			stream->buf_data += bytes;
			return nmemb;
		} else {
			memcpy(stream->curr_ptr, ptr, space_left);
			stream->curr_ptr += space_left;
			bytes -= space_left;
			ptr += space_left;
			write_nmemb += space_left / size;
			stream->buf_data += space_left;
		}

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
		if (stream->last_op == WRITE_OP) {
			so_fflush(stream); /* TODO */
		}
		stream->last_op = NO_OP;
		ret = lseek(stream->fd, offset, whence);
		if (ret < 0) {
			return -1;
		}
		stream->file_pos = ret;
		return 0;
	}
	return -1;
}

long so_ftell(SO_FILE *stream)
{
	if (stream) {
		return stream->file_pos;
	}
	return -1;
}

int so_feof(SO_FILE *stream)
{
	size_t size, curr_size;

	if (stream) {
		curr_size = lseek(stream->fd, 0, SEEK_CUR);
		size = lseek(stream->fd, 0, SEEK_END);
		if (size < stream->file_pos) {
			return SO_EOF;
		}
		lseek(stream->fd, curr_size, SEEK_SET);
		return 0;
	}
	return SO_EOF;
}

int so_ferror(SO_FILE *stream)
{
	return stream ? stream->error : SO_EOF;
}

SO_FILE *so_popen(const char *command, const char *type)
{
	if (command && type) {
		return NULL;
	}
	return NULL;
}

int so_pclose(SO_FILE *stream)
{
	if (stream) {
		return 0;
	}
	return SO_EOF;
}

size_t loop_read(int fd, void *buf, size_t count)
{
	size_t ret = 1, totalRead = 0;

	while (ret > 0 && count > 0) {
		ret = read(fd, buf, count);
		buf += ret;
		count -= ret;
		totalRead += ret;
	}
	return (ret == 0) ? -totalRead : totalRead;
}

size_t loop_write(int fd, void *buf, size_t count)
{
	size_t ret = 1, totalWritten = 0;

	while (ret > 0 && count > 0) {
		ret = write(fd, buf, count);
		buf += ret;
		count -= ret;
		totalWritten += ret;
	}
	return (ret == 0) ? -totalWritten : totalWritten;
}
