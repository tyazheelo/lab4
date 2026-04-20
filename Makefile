CC = gcc
CFLAGS = -Wall -I./include
LDFLAGS = -lsqlite3
COV_FLAGS = -fprofile-arcs -ftest-coverage

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

test-coverage: clean-coverage
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(COV_FLAGS) $(TEST_AUTH_SRC) -o $(TEST_AUTH) $(LDFLAGS)
	./$(TEST_AUTH)
	gcov *.c

clean:
	rm -rf $(BIN_DIR)
	rm -f $(DATA_DIR)/test.db

clean-coverage:
	rm -f *.gcda *.gcno *.gcov

.PHONY: all run test-auth test-coverage clean clean-coverage