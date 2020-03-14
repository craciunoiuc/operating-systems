CC = gcc
CFLAGS = -std=c89 -Wall -Wextra -g -O0 -fPIC -c

build: so_stdio.o
	$(CC) -shared so_stdio.o -o libso_stdio.so

so_stdio.o: so_stdio.c
	$(CC) $(CFLAGS) so_stdio.c

clean:
	rm *.o *.so

