#include "./util/so_stdio.h"

SO_FILE *so_fopen(const char *pathname, const char *mode)
{
	SO_FILE *file_ptr = malloc(sizeof(SO_FILE));
	if (file_ptr == NULL)
		exit(12);
	file_ptr->fd       = 0;
	file_ptr->buf_size = 0;
	file_ptr->flags    = 0;
	file_ptr->wait     = 0;
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

/* TODO Intercalat */
int so_fgetc(SO_FILE *stream)
{
	char buffer = -1, ret = 0;

	if (stream) {
		ret = loop_read(stream->fd, &buffer, 1);
		if (ret < 1) {
			stream->error = SO_EOF;
			return ret;
		}
		return buffer;
	}
	return SO_EOF;
}

/* TODO Intercalat */
int so_fputc(int c, SO_FILE *stream)
{
	char buffer = c, ret = 0;

	if (stream) {
		stream->curr_ptr++;
		*(stream->curr_ptr) = buffer;
		if (buffer == '\n' ||
		stream->curr_ptr - stream->buffer > stream->buf_size / 2) {
			ret = loop_write(stream->fd,
					stream->buffer,
					stream->curr_ptr - stream->buffer);
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
size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	return 0;
}

/* TODO */
size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	return 0;
}

/* TODO */
int so_fflush(SO_FILE *stream)
{
	return 0;
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
	size_t ret = 1;

	while (ret > 0 && count > 0) {
		ret = read(fd, buf, count);
		buf += ret;
		count -= ret;
	}
	return (ret == 0) ? SO_EOF : 0;
}

char loop_write(int fd, void *buf, size_t count)
{
	size_t ret = 1;

	while (ret > 0 && count > 0) {
		ret = write(fd, buf, count);
		buf += ret;
		count -= ret;
	}
	return (ret == 0) ? SO_EOF : 0;
}
