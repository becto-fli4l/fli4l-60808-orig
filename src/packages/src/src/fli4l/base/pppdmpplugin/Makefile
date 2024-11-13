#
# Makefile for the mp-plugin for pppd(8).
#

ifeq ($(PPPD_BUILDDIR),)
$(error PPPD_BUILDDIR is not set)
endif

# may be overridden by the command line (e.g. for cross-compiling)
CC      = gcc
CFLAGS  = -O2
LDFLAGS =

# may not be overridden
COPT    = -fPIC -Wall -Wextra -Werror -pedantic -I $(PPPD_BUILDDIR)
PLUGIN  = mp-plugin.so
SOURCES = $(wildcard *.c)
OBJECTS = $(patsubst %.c,%.o,$(SOURCES))

all: $(PLUGIN)

$(PLUGIN): $(OBJECTS)
	$(CC) -o $@ -shared $(LDFLAGS) $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) $(COPT) -c -o $@ $<

clean:
	rm -f $(PLUGIN) $(OBJECTS)

.PHONY: all clean
