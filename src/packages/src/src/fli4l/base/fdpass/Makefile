# $Id$

CC = gcc
CFLAGS = -Wall -Wextra

PROGS = fdrecv fdsend

all: $(PROGS)
clean:
	rm -f $(PROGS) $(patsubst %,%.o,$(PROGS))

.PHONY: all clean
