#----------------------------------------------------------------------------
#  Makefile - Makefile for building libglob
#
#  Copyright (c) 2014-2016 The fli4l team
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

LIB := libglob
libglob_VERSION = 98ca2c063cfe019b673fd0ce81448c919e9c989c
libglob_ARCHIVE = glob-$(libglob_VERSION).tar.gz
libglob_TARGET  = libglob/src/gllib/libglob.a
libglob_CONF    = libglob/src/config.h

include Makefile.lib

COPTS += -I libglob/src/include

libglob_CFLAGS = -O2 $(MARCH) $(MTUNE) -D_FILE_OFFSET_BITS=64

$(libglob_CONF): | libglob/src
	@echo "  CONF   libglob" >&2
	cd libglob/src && \
		CFLAGS="$(libglob_CFLAGS)" ./configure --build=$(CHOST) --host=$(CTARGET) \
			--disable-shared --disable-rpath --disable-threads

$(libglob_TARGET): $(libglob_CONF)
	@echo "  MAKE   libglob" >&2
	$(MAKE) -C libglob/src CC=$(CC) CFLAGS="$(libglob_CFLAGS)"
	$(AR) rs $@ libglob/src/gllib/*.o
	mkdir libglob/src/include
	cp libglob/src/gllib/glob.h libglob/src/gllib/glob-libc.h libglob/src/include
