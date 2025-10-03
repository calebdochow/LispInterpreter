CC = gcc
CFLAGS = -Wall -Wextra -g

# Object files
OBJS = Sprint123.o test.o

# Target executable
TARGET = testcases

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) -lm

Sprint123.o: Sprint123.c sexpr.h
	$(CC) $(CFLAGS) -c Sprint123.c

test.o: test.c sexpr.h
	$(CC) $(CFLAGS) -c test.c

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
