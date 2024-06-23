CC = c++
FLAGS = -Wall -Wextra -Werror -std=c++20 -Wconversion -pedantic $(NIX_CFLAGS_COMPILE)
INCLUDE = include
TEST = test
BIN = bin

all: test

ring: $(INCLUDE)/ring/*.hpp
	$(CC) $(FLAGS) -fsyntax-only  $(SRC)/*

$(BIN)/test_queue: $(INCLUDE)/ring/*.hpp $(TEST)/queue.cpp
	$(CC) $(FLAGS) -o $(BIN)/test_queue $(TEST)/queue.cpp

$(BIN)/test_sync_queue: $(INCLUDE)/ring/*.hpp $(TEST)/sync_queue.cpp
	$(CC) $(FLAGS) -pthread -o $(BIN)/test_sync_queue $(TEST)/sync_queue.cpp 

test: $(BIN)/test_queue $(BIN)/test_sync_queue
	./$(BIN)/test_queue && ./$(BIN)/test_sync_queue

clean:
	rm -f $(BIN)/*
