#include "./util/so_stdio.h"

#include <stdio.h>

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
	file_ptr->curr_ptr = NULL;
	file_ptr->buffer   = NULL;
	
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
						file_ptr->flags, 0644);
				
			} else {
				file_ptr->flags = O_WRONLY | O_CREAT | O_TRUNC;
				file_ptr->fd = open(pathname,
						file_ptr->flags, 0644);
			}
			break;
		}

		case 'a': {
			if (mode[1] == '+') {
				file_ptr->flags = O_RDWR | O_APPEND | O_CREAT;
				file_ptr->fd = open(pathname,
						file_ptr->flags, 0644);

			} else {
				file_ptr->flags = O_WRONLY | O_CREAT | O_APPEND;
				file_ptr->fd = open(pathname,
						file_ptr->flags, 0644);
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

	if (stream) {
		ret = close(stream->fd);
		if (ret != 0) {
			stream->error = 1;
			return SO_EOF;
		}
		if (stream->buffer) {
			free(stream->buffer);
		}
		free(stream);
		return 0;
	}
	return SO_EOF;
}

int so_fileno(SO_FILE *stream)
{
	return stream ? stream->fd : SO_EOF;
}

/* TODO Intercalat caz cu RW - poate 2 buffere */
int so_fgetc(SO_FILE *stream)
{
	int ret = 0;

	if (stream) {
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

/* TODO Intercalat caz cu RW - poate 2 buffere */
size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	int ret = 0;
	size_t bytes = size * nmemb;
	size_t read_nmemb = 0;

	if (stream) {
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
	return 0;
}

/* TODO Intercalat */
int so_fputc(int c, SO_FILE *stream)
{
	char buffer = c;
	int  ret = 0;

	if (stream) {
		*(stream->curr_ptr) = buffer;
		stream->curr_ptr++;
		stream->buf_data++;
		if (buffer == '\n' ||
		stream->curr_ptr - stream->buffer >= stream->buf_size) {
			ret = loop_write(stream->fd, stream->buffer,
				stream->buf_data);
			if (ret == SO_EOF) {
				stream->error = SO_EOF;
				return SO_EOF;
			}
			stream->curr_ptr = stream->buffer;
		}
		return c;
	}
	return SO_EOF;
}

/* TODO */
size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	return 0;
}

int so_fflush(SO_FILE *stream)
{
	char ret;

	if (stream) {
		ret = loop_write(stream->fd, stream->buffer,
				stream->curr_ptr - stream->buffer);
		stream->curr_ptr = stream->buffer;
		return ret;
	}
	return SO_EOF;
}

/* TODO */
int so_fseek(SO_FILE *stream, long offset, int whence)
{
	return 0;
}

/* TODO */
long so_ftell(SO_FILE *stream)
{
	return 0;
}

/* TODO */
SO_FILE *so_popen(const char *command, const char *type)
{
	return NULL;
}

/* TODO */
int so_pclose(SO_FILE *stream)
{
	return NULL;
}

int so_feof(SO_FILE *stream)
{
	char buffer = -1, ret = 0;

	if (stream) {
		ret = read(stream->fd, &buffer, 1);
		if (ret < 1) {
			return SO_EOF;
		}
		return lseek(stream->fd, -1, SEEK_CUR);
	}
	return SO_EOF;
}

int so_ferror(SO_FILE *stream)
{
	return stream ? stream->error : SO_EOF;
}

char loop_read(int fd, void *buf, size_t count)
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

char loop_write(int fd, void *buf, size_t count)
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
