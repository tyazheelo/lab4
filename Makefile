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

# ========== БАЗА ДАННЫХ ==========
db-init:
	mkdir -p $(DATA_DIR)
	sqlite3 $(DATA_DIR)/ippodrom.db < $(DATA_DIR)/schema.sql
	@echo "✅ Database initialized from schema.sql"

db-drop:
	rm -f $(DATA_DIR)/ippodrom.db
	rm -f $(DATA_DIR)/test.db

db-reset: db-drop db-init
	@echo "✅ Database reset complete"

# ========== ОСНОВНАЯ ПРОГРАММА ==========
all: $(TARGET)

$(TARGET): $(SRC)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

run: all
	./$(TARGET)

# ========== ТЕСТЫ ==========
test-auth: db-test $(TEST_AUTH)
	./$(TEST_AUTH)

$(TEST_AUTH): $(TEST_AUTH_SRC)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test-coverage: clean-coverage db-test
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(COV_FLAGS) $(TEST_AUTH_SRC) -o $(TEST_AUTH) $(LDFLAGS)
	./$(TEST_AUTH)
	gcov $(SRC_DIR)/auth.c
	@echo "Coverage report: auth.c.gcov"

# ========== ОЧИСТКА ==========
clean:
	rm -rf $(BIN_DIR)
	rm -f $(DATA_DIR)/test.db

clean-coverage:
	rm -f *.gcda *.gcno *.gcov

clean-all: clean clean-coverage db-drop

.PHONY: all run test-auth test-coverage clean clean-coverage clean-all db-init db-test db-drop db-reset