CC = gcc
CFLAGS = -Wall -Wextra

all: empty part1

empty: empty.c
	$(CC) $(CFLAGS) -o $@ $<

part1: part1.c
	$(CC) $(CFLAGS) -o $@ $<

trace: all
	strace -o empty.trace ./empty
	strace -o part1.trace ./part1

clean:
	rm -f empty part1

.PHONY: all trace clean
