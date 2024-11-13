#----------------------------------------------------------------------------
#  Makefile - Makefile for building liblzma
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

LIB := liblzma
liblzma_VERSION = 5.0.4
liblzma_ARCHIVE = xz-$(liblzma_VERSION).tar.bz2
liblzma_TARGET  = liblzma/src/src/liblzma/.libs/liblzma.a
liblzma_CONF    = liblzma/src/config.h

include Makefile.lib

COPTS += -I liblzma/src/src/liblzma/api

liblzma_CFLAGS = -O2 $(MARCH) $(MTUNE)

$(liblzma_CONF): | liblzma/src
	@echo "  CONF   liblzma" >&2
	cd liblzma/src && \
		CFLAGS="$(liblzma_CFLAGS)" ./configure --build=$(CHOST) --host=$(CTARGET) \
			--disable-shared --disable-rpath --disable-threads --enable-largefile \
			--disable-debug --disable-xz --disable-xzdec --disable-lzmadec \
			--disable-lzmainfo --disable-lzma-links --disable-scripts

$(liblzma_TARGET): $(liblzma_CONF)
	@echo "  MAKE   liblzma" >&2
	$(MAKE) -C liblzma/src
