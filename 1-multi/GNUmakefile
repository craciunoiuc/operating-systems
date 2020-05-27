CC = gcc
CFLAGS = -Wall -g -O0 -std=c89

all: so-cpp

build: so-cpp

so-cpp: main.o hashtable.o
	$(CC) $^ -o $@

main.o: hashtable.h main.c
	$(CC) $(CFLAGS) -c main.c

hashtable.o: hashtable.c
	$(CC) $(CFLAGS) -c hashtable.c

.PHONY: clean
clean:
	rm -f *.o *~ so-cpp
	
