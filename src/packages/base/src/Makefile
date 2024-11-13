#----------------------------------------------------------------------------
#  Makefile - Makefile for mkfli4l and related tools
#
#  Copyright (c) 2000-2001 - Frank Meyer
#  Copyright (c) 2001-2016 - fli4l-Team <team@fli4l.de>
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

COMP    = bz2
NAME    = fli4l-tools
ARCHIVE = $(NAME).tar.$(COMP)

export VERSION = $(shell cat VERSION)

ifeq ($(ARCHIVE),)
$(error Archive $(ARCHIVE) not found)
endif

all:
	mkdir -p $(NAME)
	tar xf $(ARCHIVE) -C $(NAME)
	cd $(NAME) && MAKE=$(MAKE) DEBUG=$(DEBUG) ./mkmkfli4l.sh --build ../..

clean:
	rm -rf $(NAME)

.PHONY: all clean
