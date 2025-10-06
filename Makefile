CC = gcc
CFLAGS = -I src -Wall -Wextra -g

SOURCE = src/Yisp.c
HEADER = src/sexpr.h

# --- Default target ---
all: yisp

# --- Build interpreter ---
yisp: src/main.c $(SOURCE) $(HEADER)
	$(CC) $(CFLAGS) -o yisp src/main.c $(SOURCE)

run: yisp
	./yisp

# --- Build & run tests ---
test: src/test.c $(SOURCE) $(HEADER)
	$(CC) $(CFLAGS) -o test src/test.c $(SOURCE)
	./test

# --- Cleanup ---
clean:
	rm -f yisp test *.o
