#----------------------------------------------------------------------------
#  Makefile - Makefile for building libbz2
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

LIB := libbz2
libbz2_VERSION = 1.0.6
libbz2_ARCHIVE = bzip2-$(libbz2_VERSION).tar.gz
libbz2_TARGET  = libbz2/src/libbz2.a

include Makefile.lib

COPTS += -I libbz2/src

libbz2_CFLAGS = -O2 $(MARCH) $(MTUNE) -D_FILE_OFFSET_BITS=64
$(libbz2_TARGET):
	@echo "  MAKE   libbz2" >&2
	$(MAKE) -C libbz2/src CC=$(CC) AR=$(AR) RANLIB=$(RANLIB) CFLAGS="$(libbz2_CFLAGS)" $(notdir $@)
