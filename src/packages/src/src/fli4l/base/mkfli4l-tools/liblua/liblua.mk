#----------------------------------------------------------------------------
#  Makefile - Makefile for building liblua
#
#  Copyright (c) 2018 The fli4l team
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

LIB := liblua
liblua_VERSION = 5.3.5
liblua_ARCHIVE = lua-$(liblua_VERSION).tar.gz
liblua_TARGET  = liblua/src/src/liblua.a
liblua_LIBS    = -lm

include Makefile.lib

COPTS += -I liblua/src/src

ifeq ($(OS),mingw32)
PLAT = mingw
else ifneq ($(filter darwin%,$(OS)),)
PLAT = macosx
else
PLAT = $(OS)
endif

liblua_CFLAGS = -O2 $(MARCH) $(MTUNE) -D_FILE_OFFSET_BITS=64 -DLUA_32BITS -DLUA_COMPAT_5_1 -DLUA_COMPAT_5_2
$(liblua_TARGET):
	@echo "  MAKE   liblua" >&2
	$(MAKE) -C liblua/src/src CC=$(CC) AR="$(AR) rcu" RANLIB=$(RANLIB) MYCFLAGS="$(liblua_CFLAGS)" PLAT=$(PLAT) $(notdir $@)
