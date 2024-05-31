# Makefile

CC=gcc
CFLAGS=-Wall

all: ep3

ep3: ep3.o
	$(CC) $(CFLAGS) -o ep3 ep3.o

ep3.o: ep3.c ep3.h
	$(CC) $(CFLAGS) -c ep3.c

clean:
	rm -f *.o ep3
