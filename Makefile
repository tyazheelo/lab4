CC = gcc
CFLAGS = -Wall -I./include
LDFLAGS = -lsqlite3

SRC_DIR = src
TEST_DIR = tests
BIN_DIR = bin
DATA_DIR = data

SRC = $(wildcard $(SRC_DIR)/*.c)
TARGET = $(BIN_DIR)/ippodrom

TEST_AUTH_SRC = $(TEST_DIR)/test_auth.c $(SRC_DIR)/auth.c
TEST_AUTH = $(BIN_DIR)/test_auth

all: $(TARGET)

$(TARGET): $(SRC)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

run: all
	./$(TARGET)

test-auth: $(TEST_AUTH)
	./$(TEST_AUTH)

$(TEST_AUTH): $(TEST_AUTH_SRC)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf $(BIN_DIR)

clean-coverage:
	rm -f *.gcda *.gcno *.gcov

.PHONY: all run test-auth clean clean-coverage