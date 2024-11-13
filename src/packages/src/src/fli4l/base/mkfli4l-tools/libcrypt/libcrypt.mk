#----------------------------------------------------------------------------
#  Makefile - Makefile for building libcrypt
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

LIBCRYPT_SOURCES := $(wildcard libcrypt/*.c)
LIBCRYPT_OBJECTS := $(patsubst %.c,%.o,$(filter %.c,$(LIBCRYPT_SOURCES)))
LIBCRYPT_DEPS    := $(patsubst %.c,%.P,$(filter %.c,$(LIBCRYPT_SOURCES)))

$(LIBCRYPT_OBJECTS): COPTS += -std=c99 -D_FORTIFY_SOURCE=0
$(LIBCRYPT_OBJECTS): CWARN += -Wno-missing-prototypes -Wno-sign-compare -Wno-strict-aliasing

libcrypt/libcrypt.a: $(LIBCRYPT_OBJECTS)
	@echo "  AR     $@" >&2
	$(AR) rcs $@ $^

libcrypt-clean:
	$(RM) $(LIBCRYPT_OBJECTS) libcrypt/libcrypt.a

libcrypt-distclean:
	$(RM) $(LIBCRYPT_DEPS)

clean: libcrypt-clean
distclean: libcrypt-distclean

-include $(LIBCRYPT_DEPS)

.PHONY: libcrypt-clean libcrypt-distclean
