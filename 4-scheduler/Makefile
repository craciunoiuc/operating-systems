CC=gcc
CFLAGS=-Wall -Wextra -g

build: tema_lin.o priorityqueue.o
	$(CC) -shared tema_lin.o priorityqueue.o -o libscheduler.so

tema_lin.o: tema_lin.c
	$(CC) $(CFLAGS) -D __linux__ -fPIC -c tema_lin.c -pthread

priorityqueue.o: priorityqueue.c
	$(CC) $(CFLAGS) -fPIC -c priorityqueue.c

move: build
	mv libscheduler.so ./checker-lin/libscheduler.so

clean:
	rm -rf tema_lin.o priorityqueue.o libscheduler.so