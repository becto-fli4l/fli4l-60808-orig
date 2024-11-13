#----------------------------------------------------------------------------
#  Makefile - Makefile for building libz
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

LIB := libz
libz_VERSION = 1.2.7
libz_ARCHIVE = zlib-$(libz_VERSION).tar.gz
libz_TARGET  = libz/src/libz.a
ifneq ($(OS),mingw32)
libz_CONF    = libz/src/configure.log
endif

include Makefile.lib

COPTS += -I libz/src

libz_CFLAGS   = -O2 $(MARCH) $(MTUNE)
libz_MAKE_ENV = CC=$(CC) AR=$(AR)

# zlib's configure script is not particularly suited for cross compiling,
# so we have to correct its decision about ARFLAGS on darwin
ifneq ($(filter darwin%,$(OS)),)
libz_MAKE_ENV += ARFLAGS=rc
endif

ifneq ($(OS),mingw32)
$(libz_CONF): | libz/src
	@echo "  CONF   libz" >&2
	cd libz/src && CHOST=$(CTARGET) CFLAGS="$(libz_CFLAGS)" ./configure --static
endif

ifeq ($(OS),mingw32)
$(libz_TARGET):
	@echo "  MAKE   libz" >&2
	$(MAKE) -C libz/src -f win32/Makefile.gcc PREFIX=$(CROSS) CFLAGS="$(libz_CFLAGS)" $(notdir $@)
else
$(libz_TARGET): $(libz_CONF)
	@echo "  MAKE   libz" >&2
	$(MAKE) -C libz/src $(libz_MAKE_ENV) $(notdir $@)
endif
