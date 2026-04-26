CC = gcc
CFLAGS = -Wall -I./include -Wno-format-truncation
LDFLAGS = -lsqlite3 -lm

SRC = $(wildcard src/*.c)
TARGET = bin/ippodrom

all:
	mkdir -p bin
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

run: all
	./$(TARGET)

clean:
	rm -rf bin

.PHONY: all run clean
