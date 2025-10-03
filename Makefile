CC = gcc
CFLAGS = -Wall -Wextra -g

#Object files
COMMON_OBJS = src/sexpr.o

#Repl
REPL_OBJS = src/Yisp.o $(COMMON_OBJS)
REPL_TARGET = repl

#Test
TEST_OBJS = src/test.o $(COMMON_OBJS)
TEST_TARGET = testcases

all: $(REPL_TARGET) $(TEST_TARGET)

#repl
$(REPL_TARGET): $(REPL_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lm

#test environment
$(TEST_TARGET): $(TEST_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lm

#compile 
src/%.o: src/%.c src/%.h
	$(CC) $(CFLAGS) -c $<

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $<

#make repl or make tests
run-repl: $(REPL_TARGET)
	./$(REPL_TARGET)

run-test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f src/*.o $(REPL_TARGET) $(TEST_TARGET)
