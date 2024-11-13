#----------------------------------------------------------------------------
#  Makefile - Makefile for building libregex
#
#  Copyright (c) 2012-2016 The fli4l team
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

LIB := libregex
libregex_VERSION = 3a9002d3cc63da7110f133b1040d2d2b0aad8305
libregex_ARCHIVE = libregex-$(libregex_VERSION).tar.bz2
libregex_TARGET  = libregex/src/libregex.a
libregex_SOURCES = libregex/src/regex.c
libregex_OBJECTS = $(patsubst %.c,%.o,$(libregex_SOURCES))

include Makefile.lib

COPTS += -D_Restrict_arr_ -I libregex/src

$(libregex_SOURCES): libregex/src

$(libregex_OBJECTS): COPTS += -Dinline=__inline -I libregex/config-$(OS) -I libregex/config
$(libregex_OBJECTS): CWARN :=

$(libregex_TARGET): $(libregex_OBJECTS)
	@echo "  AR     $@" >&2
	$(AR) rcs $@ $(libregex_OBJECTS)
