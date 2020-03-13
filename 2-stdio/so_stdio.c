#include "./util/so_stdio.h"

SO_FILE *so_fopen(const char *pathname, const char *mode)
{
	SO_FILE *file_ptr = malloc(sizeof(SO_FILE));
	file_ptr->fd       = 0;
	file_ptr->buf_size = 0;
	file_ptr->flags    = 0;
	file_ptr->wait     = 0;
	file_ptr->curr_ptr = NULL;
	file_ptr->buf_size = NULL;
	if (file_ptr == NULL)
		return NULL;
	
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
	return file_ptr;
}

int main() {

	return 0;
}
