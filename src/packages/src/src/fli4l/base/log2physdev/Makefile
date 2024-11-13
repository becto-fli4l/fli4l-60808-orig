CROSS = i686-pc-mingw32-
CC = $(CROSS)gcc
CFLAGS = -Wall -Os -s

log2physdev.exe: log2physdev.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f log2physdev.exe log2physdev.o

.PHONY: clean
