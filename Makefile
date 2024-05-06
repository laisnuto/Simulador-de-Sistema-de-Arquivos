CC = gcc

CFLAGS = -Wall -lpthread -std=gnu99
LIBS = -lm

TARGET = ep2

OBJS = ep2.o

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

ep2.o: ep2.c ep2.h
	$(CC) $(CFLAGS) -c ep2.c

clean:
	rm -f $(TARGET) $(OBJS)

run: $(TARGET)
	./$(TARGET) 100 20  

debug: CFLAGS += -g
debug: $(TARGET)
