CC = gcc

CFLAGS = -Wall -lpthread

TARGET = ep2

OBJS = ep2.o

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

ep2.o: ep2.c ep2.h
	$(CC) $(CFLAGS) -c ep2.c

clean:
	rm -f $(TARGET) $(OBJS)

run: $(TARGET)
	./$(TARGET) 100 20  # Example arguments, replace 100 and 20 with appropriate values

debug: CFLAGS += -g
debug: $(TARGET)
