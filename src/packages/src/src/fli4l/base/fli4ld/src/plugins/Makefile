#----------------------------------------------------------------------------
#  Makefile - Makefile for fli4ld plug-ins
#
#  Copyright (c) 2015 - fli4l-Team <team@fli4l.de>
#
#  This file is part of fli4l.
#
#  fli4l is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  fli4l is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with fli4l.  If not, see <http://www.gnu.org/licenses/>.
#
#  Last Update: $Id$
#----------------------------------------------------------------------------

ifeq ($(PLUGIN),)
$(error You have to set PLUGIN to the plug-in's name)
endif

CXX     = g++
CFLAGS  = -O2
LDFLAGS =
LDLIBS  = -pthread -ldl
COPTS   = -std=c++17 -pthread -D_XOPEN_SOURCE=700 -fPIC
CWARN   = -Wall -Wextra -Werror -Wno-long-long -pedantic

TARGET  = $(PLUGIN).so
SOURCES = $(wildcard *.cc)
OBJECTS = $(patsubst %.cc,%.o,$(SOURCES))

$(TARGET): $(OBJECTS)
	@echo "  LD     $@" >&2
	@$(CXX) $(LDFLAGS) -shared -o $@ $^ $(LDLIBS)

all: $(TARGET)

clean:
	rm -f $(TARGET)
	rm -f $(OBJECTS)

distclean: clean
	rm -f $(patsubst %.cc,%.P,$(SOURCES))

docs:
	rm -rf docs
	mkdir -p docs
	doxygen

install: install-data install-exec

install-data:

install-exec:
	install -D -m 755 -t $(DESTDIR)/usr/lib/fli4ld/ $(TARGET)

-include $(patsubst %.cc,%.P,$(SOURCES))

define MAKEDEPEND
	cp $(1).d $@
	sed -i -e 's,$(1).o *:,$(1).o $@:,' $@
	sed -e 's/#.*//;s/^[^:]*: *//;s/ *\\$$//;/^$$/d;s/$$/:/;s/^ *//' < $(1).d >> $@
	rm -f $(1).d
endef

%.P: %.cc
	@$(CXX) $(CFLAGS) $(COPTS) -MM -MG -w -MF $*.d -MQ $*.o -c $<
	@$(call MAKEDEPEND,$*)

%.o: %.cc
	@echo "  CXX    $@" >&2
	@$(CXX) $(CFLAGS) $(COPTS) $(CWARN) -o $@ -c $<

.PHONY: all clean distclean docs install install-data install-exec
