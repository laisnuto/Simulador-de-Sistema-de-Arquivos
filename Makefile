CC=gcc
CFLAGS=-Wall -D_GNU_SOURCE
LDFLAGS=-lpthread -lreadline -lm

EXEC1=ep1
EXEC2=newsh

SRCS1=ep1.c
OBJS1=$(SRCS1:.c=.o)

SRCS2=newsh.c
OBJS2=$(SRCS2:.c=.o)

all: $(EXEC1) $(EXEC2)

$(EXEC1): $(OBJS1)
	$(CC) -o $(EXEC1) $(OBJS1) $(CFLAGS) $(LDFLAGS)

$(EXEC2): $(OBJS2)
	$(CC) -o $(EXEC2) $(OBJS2) $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(OBJS1) $(EXEC1) $(OBJS2) $(EXEC2)

ep1.o: ep1.c ep1.h

newsh.o: newsh.c newsh.h
