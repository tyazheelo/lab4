CC = gcc
CFLAGS = -Wall -I./include
LDFLAGS = -lsqlite3

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