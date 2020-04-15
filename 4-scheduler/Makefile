CC=gcc
CFLAGS=-Wall -Wextra -g

build: tema_lin.o
	$(CC) -shared tema_lin.o -o libscheduler.so

tema_lin.o: tema_lin.c
	$(CC) $(CFLAGS) -D __linux__ -fPIC -c tema_lin.c -lpthread

move: build
	mv libscheduler.so ./checker-lin/libscheduler.so

clean:
	rm -rf tema_lin.o libscheduler.so