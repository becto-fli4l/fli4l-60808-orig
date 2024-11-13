#----------------------------------------------------------------------------
#  telmond/Makefile
#
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  Last Update:    $Id$
#----------------------------------------------------------------------------

CC    = $(CROSS)gcc
AR    = $(CROSS)ar
LEX   = flex
YACC  = bison

ARCH     = i486
PLATFORM = linux
CROSS    =
STATIC   =

# special handling for mingw32 platform...
ifeq ($(PLATFORM),mingw32)
PREFIX =
else
PREFIX = $(ARCH)-$(PLATFORM)-
endif

CFLAGS  = -O2 -march=$(ARCH)
ifeq ($(VERSION),)
DATE    = $(shell date +%Y-%m-%d)
else
DATE    = Revision: $(VERSION)
endif
COPTS   = -I . -DPLATFORM_$(PLATFORM) -DYYDEBUG=1 -DYY_NO_INPUT=1 -DDATE="\"$(DATE)\"" -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0
CWARN   = -Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes # -pedantic
LFLAGS  = -d --nounput
YFLAGS  =
LDFLAGS = $(STATIC) -s
LDLIBS  = -lcapi20

ifeq ($(PLATFORM),mingw32)
EXEEXT = .exe
else
EXEEXT =
endif

VERBOSE = @

all:
clean:
cleanall:

include telmond.mk

%.o: %.c
	@echo "  CC    $@"
	$(VERBOSE)$(CC) $(CFLAGS) $(COPTS) -MD -w -o $@ -c $<
	@cp $*.d $*.P
	@sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
		-e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P
	@rm -f $*.d
	$(VERBOSE)$(CC) $(CFLAGS) $(COPTS) $(CWARN) -o $@ -c $<

%.c: %.l
	@echo "  LEX   $@"
	$(VERBOSE)$(LEX) $(LFLAGS) -o $@ $<

%.c: %.y
	@echo "  YACC  $@"
	$(VERBOSE)$(YACC) $(YFLAGS) -d -o $@ $<

.PHONY: all clean cleanall
