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
				file_ptr->fd = CreateFile(
						pathname,
						GENERIC_READ | GENERIC_WRITE,
						FILE_SHARE_READ,
						NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,
						NULL
						);
				file_ptr->flags |= OPEN_EXISTING |
						GENERIC_READ | GENERIC_WRITE;
			} else {
				file_ptr->fd = CreateFile(
							pathname,
							GENERIC_READ,
							FILE_SHARE_READ,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL
							);
				file_ptr->flags |= OPEN_EXISTING |
						GENERIC_READ;
			}
			break;
		}

		case 'w': {
			if (mode[1] == '+') {
				file_ptr->flags = CREATE_ALWAYS |
						GENERIC_WRITE | GENERIC_READ;
				file_ptr->fd = CreateFile(
					pathname,
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					CREATE_ALWAYS,
					FILE_ATTRIBUTE_NORMAL,
					NULL
					);		
			} else {
				file_ptr->flags = CREATE_ALWAYS |
						GENERIC_WRITE;
				file_ptr->fd = CreateFile(
							pathname,
							GENERIC_WRITE,
							FILE_SHARE_WRITE,
							NULL,
							CREATE_ALWAYS,
							FILE_ATTRIBUTE_NORMAL,
							NULL
							);
			}
			break;
		}

		case 'a': {
			if (mode[1] == '+') {
				file_ptr->flags = GENERIC_WRITE |
						GENERIC_READ | OPEN_ALWAYS;
				file_ptr->fd = CreateFile(
					pathname,
					GENERIC_WRITE | GENERIC_READ,
					FILE_SHARE_WRITE | FILE_SHARE_READ,
					NULL,
					OPEN_ALWAYS,
					FILE_ATTRIBUTE_NORMAL,
					NULL
				);

			} else {
				file_ptr->flags = GENERIC_WRITE | OPEN_ALWAYS;
				file_ptr->fd = CreateFile(
							pathname,
							GENERIC_WRITE,
							FILE_SHARE_WRITE,
							NULL,
							OPEN_ALWAYS,
							FILE_ATTRIBUTE_NORMAL,
							NULL
							);
			}
			break;
		}

		default: {
			free(file_ptr);
			return NULL;
		}
	}
	if (file_ptr->fd == INVALID_HANDLE_VALUE) {
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
		ret = CloseHandle(stream->fd);
		if (stream->buffer)
			free(stream->buffer);
		free(stream);
		if (ret == 0)
			return SO_EOF;
		return fail ? SO_EOF : 0;
	}
	return SO_EOF;
}

/* Returns the handle in the stream */
HANDLE so_fileno(SO_FILE *stream)
{
	return stream->fd;
}

/* Buffers data from the file into the stream and returns one character */
int so_fgetc(SO_FILE *stream)
{
	int ret = 0, bRet = 0;

	if (stream) {
		stream->last_op = READ_OP;
		stream->file_pos++;
		if (stream->curr_ptr - stream->buffer >= stream->buf_size) {
			stream->curr_ptr = stream->buffer;
			stream->buf_data = 0;
		}
		if (stream->curr_ptr - stream->buffer >= stream->buf_data) {
			bRet = ReadFile(
				stream->fd,
				stream->curr_ptr,
				stream->buf_size - stream->buf_data,
				&ret,
				NULL
				);
			if (bRet == 0 || ret == 0) {
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

/* Copies buffer chunks into ptr, and gets data from the disk, if needed */
size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	char *cast_ptr = ptr;
	int ret = 0, bRet = 0;
	size_t bytes = size * nmemb;
	size_t read_nmemb = 0;

	if (stream) {
		stream->last_op = READ_OP;
		stream->file_pos += bytes;
		if (stream->buf_data == 0) {
			stream->curr_ptr = stream->buffer;
		} else {
			if (stream->buf_data > bytes) {
				memcpy(cast_ptr, stream->curr_ptr, bytes);
				stream->curr_ptr += bytes;
				stream->buf_data -= bytes;
				return nmemb;
			}
			memcpy(cast_ptr, stream->curr_ptr, stream->buf_data);
			cast_ptr += stream->buf_data;
			nmemb -= stream->buf_data / size;
			bytes -= stream->buf_data;
			stream->curr_ptr = stream->buffer;
			read_nmemb += stream->buf_data / size;
			stream->buf_data = 0;
		}
		while (bytes > 0) {
			bRet = ReadFile(
					stream->fd,
					stream->curr_ptr,
					stream->buf_size - stream->buf_data,
					&ret,
					NULL
					);
			if (ret == 0 || bRet == 0) {
				stream->error = SO_EOF;
				return read_nmemb;
			}
			if (ret < 0) {
				stream->error = SO_EOF;
				return 0;
			}
			stream->buf_data += ret;
			if (bytes > stream->buf_data) {
				memcpy(cast_ptr, stream->curr_ptr,
							stream->buf_data);
				cast_ptr += stream->buf_data;
				bytes -= stream->buf_data;
				stream->curr_ptr = stream->buffer;
				read_nmemb += stream->buf_data / size;
				stream->buf_data = 0;
			} else {
				memcpy(cast_ptr, stream->curr_ptr, bytes);
				cast_ptr += bytes;
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

/* Copies one character in the buffer and writes it if \n or buffer filled */
int so_fputc(int c, SO_FILE *stream)
{

	char buffer = c;
	int  ret = 0, bRet = 0;

	if (stream) {
		stream->last_op = WRITE_OP;
		stream->file_pos++;
		*(stream->curr_ptr) = buffer;
		stream->curr_ptr++;
		stream->buf_data++;
		if (buffer == '\n' || stream->buf_data == stream->buf_size) {
			bRet = WriteFile(
					stream->fd,
					stream->buffer,
					stream->buf_data,
					&ret,
					NULL
					);
			if (ret == 0 || bRet == 0) {
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

/* Writes data in the buffer and copies it into the file if buffer is full */
size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	const char *cast_ptr = ptr;
	int space_left = 0, write_nmemb = 0, ret, bRet;
	int bytes = size * nmemb;

	if (stream) {
		stream->last_op = WRITE_OP;
		stream->file_pos += bytes;
		if (stream->buf_data == 0)
			stream->curr_ptr = stream->buffer;

		/* There is space in the buffer */
		space_left = stream->buf_size - stream->buf_data;
		if (space_left >= bytes) {
			memcpy(stream->curr_ptr, cast_ptr, bytes);
			stream->curr_ptr += bytes;
			stream->buf_data += bytes;
			return nmemb;
		}
		memcpy(stream->curr_ptr, cast_ptr, space_left);
		stream->curr_ptr += space_left;
		bytes -= space_left;
		cast_ptr += space_left;
		write_nmemb += space_left / size;
		stream->buf_data += space_left;

		while (bytes > 0) {
			bRet = WriteFile(
					stream->fd,
					stream->buffer,
					stream->buf_data,
					&ret,
					NULL
					);
			if (ret == 0 || bRet == 0) {
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
				memcpy(stream->curr_ptr, cast_ptr, bytes);
				stream->curr_ptr += bytes;
				stream->buf_data += bytes;
				write_nmemb += bytes / size;
				bytes = 0;
			} else {
				memcpy(stream->curr_ptr, cast_ptr, space_left);
				stream->curr_ptr += space_left;
				stream->buf_data += space_left;
				cast_ptr += space_left;
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
	int ret, bRet;

	if (stream) {
		bRet = WriteFile(
				stream->fd,
				stream->buffer,
				stream->buf_data,
				&ret,
				NULL
				);
		if (ret == 0 || bRet == 0) {
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

/* Clears the buffer and moves the cursor to the wanted position */
int so_fseek(SO_FILE *stream, long offset, int whence)
{

	long ret = 0;

	if (stream) {
		if (whence != FILE_CURRENT && whence != FILE_END &&
			whence != FILE_BEGIN)
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
		ret = SetFilePointer(
				stream->fd,
				offset,
				NULL,
				whence
				);
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
	int size, curr_size, ret;

	if (stream) {
		curr_size = SetFilePointer(
					stream->fd,
					0,
					NULL,
					FILE_CURRENT
					);
		size = SetFilePointer(
				stream->fd,
				0,
				NULL,
				FILE_END
				);
		if (curr_size ==  INVALID_SET_FILE_POINTER ||
			size ==  INVALID_SET_FILE_POINTER ||
			size < (int) stream->file_pos)
			return SO_EOF;
		ret = SetFilePointer(
				stream->fd,
				curr_size,
				NULL,
				FILE_BEGIN
				);
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

/* Not implemented. Empty body for compilation to succeed without warnings. */
SO_FILE *so_popen(const char *command, const char *type)
{
	if (command && type)
		return NULL;
	return NULL;
}

/* Not implemented. Empty body for compilation to succeed without warnings. */
int so_pclose(SO_FILE *stream)
{
	if (stream)
		return 0;
	return -1;
}

/* Small loop write implementation to guarantee data has been written */
int loop_write(HANDLE fd, char *buf, int count)
{
	int ret = 1, totalWritten = 0, bRet;

	while (ret > 0 && count > 0) {
		bRet = WriteFile(
				fd,
				buf,
				count,
				&ret,
				NULL
				);
		if (ret == 0 || bRet == 0)
			return 0;
		buf += ret;
		count -= ret;
		totalWritten += ret;
	}
	return (ret == 0) ? -totalWritten : totalWritten;
}
