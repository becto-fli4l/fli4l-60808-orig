# Makefile for imond
#
# Magic by Tobias Gruetzmacher <fli4l@portfolio16.de>
#
# Last change: $Id: Makefile 5136 2004-04-02 16:28:49Z tobig $

CFLAGS = -g -Os -fstrength-reduce -Wall
LDFLAGS = -lcrypt
DEBUG_CFLAGS = -g -Wall -DTEST
# We expect gcc, since imond is currently only useful on Linux, that should be no problem.
CC = gcc

ifeq "$(shell test -f .debug && echo 1)" "1"
  $(warning Debug build active! Do a "make clean" and then "make" to get a standard build again!)
  CFLAGS = $(DEBUG_CFLAGS)
endif

TARGETS = imond
all: $(TARGETS)

# don't use it yet!
install: imond
	cp imond ../../../opt/files/usr/local/bin/imond

debug:
	@touch .debug
	@$(MAKE) all

# Use makedepend from X (fast!) or fall back to gcc (slow!)
.depend: MKDEP=$(shell which makedepend > /dev/null && echo "makedepend -f-" || echo "$(CC) -MM")
.depend: $(wildcard *.c)
	$(MKDEP) $^ > $@

imond: main.o md5.o date.o log.o connection.o user.o options.o pppoe.o \
	isdn.o led.o
	$(CC) $(LDFLAGS) $^ -o $@

doc: HAVE_DOT=$(shell which dot > /dev/null && echo "YES" || echo "NO")
doc:
	HAVE_DOT=$(HAVE_DOT) doxygen 2>&1 | tee .doxylog
	@echo "Not documented: `grep "not documented" .doxylog | wc -l`"
	-rm -f .doxylog

clean:
	rm -Rf *.o .depend .debug $(TARGETS) doc

-include .depend

.PHONY: doc
