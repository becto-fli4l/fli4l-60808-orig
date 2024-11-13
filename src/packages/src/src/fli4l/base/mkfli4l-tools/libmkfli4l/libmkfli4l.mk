#----------------------------------------------------------------------------
#  Makefile - Makefile for building libmkfli4l
#
#  Copyright (c) 2012-2016 - fli4l-Team <team@fli4l.de>
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

LIBMKFLI4L_SOURCES_GENERATED := $(addprefix libmkfli4l/,lex$(SUFFIX).c cfg$(SUFFIX).c grammar.c grammar.h)
LIBMKFLI4L_SOURCES := $(addprefix libmkfli4l/,array.c check.c \
file.c log.c options.c parse.c regex.c \
set_var.c str.c tree.c tree_debug.c var.c) \
$(filter %.c,$(LIBMKFLI4L_SOURCES_GENERATED))
LIBMKFLI4L_OBJECTS := $(patsubst %.c,%.o,$(filter %.c,$(LIBMKFLI4L_SOURCES)))
LIBMKFLI4L_DEPS    := $(patsubst %.c,%.P,$(filter %.c,$(LIBMKFLI4L_SOURCES)))

ifeq ($(OS),mingw32)
COPTS += -isystem libmkfli4l/win32
endif

$(LIBMKFLI4L_OBJECTS): libregex/libregex.a libglob/libglob.a

libmkfli4l/libmkfli4l.a: $(LIBMKFLI4L_OBJECTS)
	@echo "  AR     $@" >&2
	$(AR) rcs $@ $^

libmkfli4l-clean:
	$(RM) $(LIBMKFLI4L_OBJECTS) libmkfli4l/libmkfli4l.a

libmkfli4l-distclean:
	$(RM) $(LIBMKFLI4L_DEPS) $(LIBMKFLI4L_SOURCES_GENERATED)

clean: libmkfli4l-clean
distclean: libmkfli4l-distclean

$(addprefix libmkfli4l/,grammar.c grammar.h): $(addprefix libmkfli4l/,defs.h tree.h tree_debug.h tree_struct.h str.h var.h log.h parse.h array.h)
$(addprefix libmkfli4l/,regex.c): $(addprefix grammar/,common.h vartype.h vartype_scanner.h vartype_parser.h)
$(addprefix libmkfli4l/,check.c): $(addprefix grammar/,common.h vardef.h vardef_scanner.h vardef_parser.h)
$(addprefix libmkfli4l/,var.c): $(addprefix grammar/,common.h varass.h varass_scanner.h varass_parser.h)

libmkfli4l/lex$(SUFFIX).o: CWARN := $(CWARN) -Wno-error=sign-compare
libmkfli4l/cfg$(SUFFIX).o: CWARN := $(CWARN) -Wno-error=sign-compare

libmkfli4l/lex$(SUFFIX).c: $(addprefix libmkfli4l/,defs.h tree.h grammar.h str.h log.h)

libmkfli4l/cfg$(SUFFIX).c: LFLAGS := $(LFLAGS) -Pcfg
libmkfli4l/cfg$(SUFFIX).c: $(addprefix libmkfli4l/,defs.h cfg.h str.h log.h)

-include $(LIBMKFLI4L_DEPS)

.PHONY: libmkfli4l-clean libmkfli4l-distclean
