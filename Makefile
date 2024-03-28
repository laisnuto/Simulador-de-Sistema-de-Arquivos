CC=gcc

CFLAGS=-Wall -lpthread -D_GNU_SOURCE

EXEC=ep1

SRCS=ep1.c
OBJS=$(SRCS:.c=.o)

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJS)

clean:
	rm -f $(OBJS) $(EXEC)

ep1.o: ep1.c ep1.h
