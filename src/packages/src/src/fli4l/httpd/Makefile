# Makefile for programs needed by the FLI4L httpd package
#
# Created:     12.09.2002 tobig
# Last change: $Id$
#
# Remember to set PATH=/usr/i386-linux-uclibc/usr/bin:$PATH if you
# want to compile with uclibc!


CC=gcc

CFLAGS=-Os -Wall -fstrength-reduce

TARGETS=bytes2read

all: $(TARGETS)
	
bytes2read: bytes2read.c
	$(CC) $(CFLAGS) -o $@ $<

install: $(TARGETS)
	cp $(TARGETS) ../../opt/files/usr/local/bin/

clean:
	rm -f $(TARGETS)
