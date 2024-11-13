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
#  Creation:    2000-05-03  fm
#  Last Update: $Id$
#----------------------------------------------------------------------------

ifeq ($(and $(CHOST),$(CTARGET),$(OS)),)
$(error Please use mkmkfli4l.sh instead of this Makefile!)
endif

CROSS  = $(CTARGET)-
CC     = $(CROSS)gcc
AR     = $(CROSS)ar
RANLIB = $(CROSS)ranlib
STRIP  = $(CROSS)strip
LEX    = flex
YACC   = bison
CP     = cp -p
RM     = rm -f
MKDIR  = mkdir -p
SED    = sed

# darwin's strip differs much from the GNU binutils' one
ifeq ($(filter darwin%,$(OS)),)
STRIP += -R .note -R .comment
endif

# use STATIC=1 to produce statically linked binaries
STATIC = 0

ifeq ($(VERSION),)
DATE = $(shell date +%Y-%m-%d)
else
DATE = Revision: $(VERSION)
endif

ifneq ($(ARCH),)
MARCH = -march=$(ARCH)
endif
ifneq ($(TUNE),)
MTUNE = -mtune=$(TUNE)
endif

CFLAGS  = $(MARCH) $(MTUNE)
LFLAGS  =
YFLAGS  =
# static linking is not supported for darwin platform due to missing static libc
ifeq ($(filter darwin%,$(OS)),)
LDFLAGS = $(if $(filter 1,$(STATIC)),-static)
else
LDFLAGS =
endif
SUFFIX  =
VALGRIND =

REPORT_DIR = reports

ifneq ($(DEBUG),1)
ifneq ($(COVERAGE),1)
CFLAGS += -O2
endif
endif
ifeq ($(DEBUG),1)
CFLAGS  += -O0 -g -DDEBUG -DYYDEBUG
LFLAGS  += -d
LDFLAGS += -g
SUFFIX   = -debug
VALGRIND = valgrind --quiet --leak-check=full --show-reachable=yes --xml=yes --xml-file=$(REPORT_DIR)/valgrind.xml
endif
ifeq ($(COVERAGE),1)
CFLAGS  += -O0 --coverage -DCOVERAGE=1
LDFLAGS += --coverage
endif
GCOV_EXCEPTIONS = libregex/.* libmkfli4l/.* grammar/.*_(scanner|parser)\.c testing/.* .*/tests/.* mkfli4ltest.c

COPTS = -DPLATFORM_$(OS) -DBUILD_DATE="\"$(DATE)\"" -I . -std=c99 -U__STRICT_ANSI__ -D_XOPEN_SOURCE=700 -D_GNU_SOURCE -D_DARWIN_C_SOURCE -D_POSIX -U_FORTIFY_SOURCE
CWARN = -Wall -Wextra -Werror -Wmissing-prototypes -Wstrict-prototypes -Wno-long-long -pedantic

EXTLIBS = libregex libglob libz libbz2 liblzma liblua
INTLIBS = libcrypt libmkfli4l

ifeq ($(OS),mingw32)
EXEEXT = .exe
else
EXEEXT =
endif

# programs to build
PROGS =

# commonly used sources (which will eventually replace libmkfli4l)
common_SOURCES = \
$(wildcard container/*.c) \
$(wildcard expression/*.c) \
$(wildcard vartype/*.c) \
$(wildcard var/*.c) \
$(wildcard package/*.c) \
$(wildcard grammar/*.c) \
$(filter %.c,$(common_SOURCES_GENERATED))
# generated sources (e.g. scanners or parsers)
common_SOURCES_GENERATED = \
$(addprefix grammar/,vartype_scanner.c vartype_scanner.h vartype_scanner.l vartype_scanner.y) \
$(addprefix grammar/,vartype_parser.c vartype_parser.h vartype_parser.l vartype_parser.y) \
$(addprefix grammar/,vardef_scanner.c vardef_scanner.h vardef_scanner.l vardef_scanner.y) \
$(addprefix grammar/,vardef_parser.c vardef_parser.h vardef_parser.l vardef_parser.y) \
$(addprefix grammar/,varass_scanner.c varass_scanner.h) \
$(addprefix grammar/,varass_parser.c varass_parser.h) \
$(addprefix grammar/,expr_scanner.c expr_scanner.h expr_scanner.l expr_scanner.y) \
$(addprefix grammar/,expr_parser.c expr_parser.h expr_parser.l expr_parser.y)
# common library dependencies (internal)
common_SOURCES_DEPS = libmkfli4l libcrypt libglob libregex
# common library dependencies (external)
common_SOURCES_LIBS =

# mkfli4l sources (configuration checking and image building)
PROGS += mkfli4l
mkfli4l_SOURCES = mkfli4l.c \
config.c buildinfo.c mk_syslinux.c \
$(addprefix elfinfo/,readelf.c) \
$(addprefix boottype/,boottype.c boottype_generic.c boottype_integrated.c boottype_attached.c) \
$(addprefix archive/,archive.c arch_type_tar.c arch_type_cpio.c squeeze$(SUFFIX).c tempfile.c) \
$(addprefix compression/,arch_comp_none.c arch_comp_gzip.c arch_comp_bz2.c arch_comp_lzma_xz.c) \
$(addprefix ziplist/,opt.c optlibs.c kernmod.c) \
$(common_SOURCES)
# mkfli4l library dependencies (internal)
mkfli4l_DEPS = $(common_SOURCES_DEPS) libz libbz2 liblzma liblua
# mkfli4l library dependencies (external)
mkfli4l_LIBS = $(common_SOURCES_LIBS)

ifneq ($(OS),mingw32)
# modules possibly containing unit tests
MODULES = elfinfo boottype archive compression ziplist \
container expression vartype var package grammar

# mkfli4ltest sources (test runner)
PROGS += mkfli4ltest
mkfli4ltest_SOURCES = mkfli4ltest.c \
$(wildcard testing/*.c) \
$(foreach group,$(MODULES),$(wildcard $(group)/tests/*.c)) \
$(common_SOURCES)
# mkfli4ltest library dependencies (internal)
mkfli4ltest_DEPS = $(common_SOURCES_DEPS)
# mkfli4ltest library dependencies (external)
mkfli4ltest_LIBS = $(common_SOURCES_LIBS)
ifeq ($(OS),linux)
mkfli4ltest_LIBS += -lrt
endif
endif

# regexp sources (regexp test utility)
PROGS += regexp
regexp_SOURCES = regexp.c $(common_SOURCES)
# regexp library dependencies (internal)
regexp_DEPS = $(common_SOURCES_DEPS)
# regexp library dependencies (external)
regexp_LIBS = $(common_SOURCES_LIBS)

# squeeze sources (shell script compressor)
PROGS += squeeze
squeeze_SOURCES = squeeze_main.c archive/squeeze$(SUFFIX).c $(common_SOURCES)
# squeeze library dependencies (internal)
squeeze_DEPS = $(common_SOURCES_DEPS)
# squeeze library dependencies (external)
squeeze_LIBS = $(common_SOURCES_LIBS)

SOURCES = $(foreach p,$(PROGS),$($(p)_SOURCES))
OBJECTS = $(patsubst %.c,%.o,$(filter %.c,$(SOURCES)))
EXECUTABLES = $(addprefix $(PREFIX),$(addsuffix $(EXEEXT),$(PROGS)))

MAKELIB = $(1)/$(1).a

define RULE
$(1)_OBJECTS = $(patsubst %.c,%.o,$(filter %.c,$($(1)_SOURCES)))

$(PREFIX)$(1)$(EXEEXT): LDLIBS := $($(1)_LIBS) $(foreach dep,$($(1)_DEPS),$($(dep)_LIBS))
$(PREFIX)$(1)$(EXEEXT): $$($(1)_OBJECTS) $(foreach dep,$($(1)_DEPS),$(call MAKELIB,$(dep)))
	@echo "  LD     $$@" >&2
	$(CC) $$(LDFLAGS) -o $$@ $$^ $$(LDLIBS)
ifeq ($(DEBUG),)
ifneq ($(STRIP),)
	@echo "  STRIP  $$@" >&2
	$(STRIP) $$@
endif
endif

$$($(1)_OBJECTS): $(foreach dep,$($(1)_DEPS),$(dep)/$(dep).a)
endef

all: $(EXECUTABLES)

clean:
	$(RM) $(OBJECTS)
	$(RM) $(EXECUTABLES)
	$(RM) $(REPORT_DIR)/*
	find . -name '*.gcda' -exec $(RM) {} +
	find . -name '*.gcno' -exec $(RM) {} +

distclean: clean
	$(RM) $(patsubst %.c,%.P,$(filter %.c,$(SOURCES)))
	$(RM) $(common_SOURCES_GENERATED)

docs:
	$(RM) -r docs
	mkdir -p docs
	doxygen

install: all
	if [ -n "$(DESTDIR)" ]; then \
		:; $(foreach e,$(EXECUTABLES),$(CP) $e $(DESTDIR)/ ;) \
	else \
		echo "DESTDIR is empty -- cannot install!"; \
		exit 1; \
	fi

ifneq ($(OS),mingw32)
check: $(PREFIX)mkfli4ltest$(EXESFX)
	find . -name '*.gcda' -exec $(RM) \{\} +
	$(RM) $(REPORT_DIR)/*
	$(MKDIR) $(REPORT_DIR)
	$(VALGRIND) ./$< --output-dir $(REPORT_DIR)
	[ -f $(REPORT_DIR)/valgrind.xml ] && extern/valgrind-distribute.sh $(REPORT_DIR) || true
	extern/gcovr --xml -r . $(patsubst %,-e '%',$(GCOV_EXCEPTIONS)) -o $(REPORT_DIR)/coverage.xml
	extern/gcovr --html --html-details -r . $(patsubst %,-e '%',$(GCOV_EXCEPTIONS)) -o $(REPORT_DIR)/coverage.html

.PHONY: check
endif

$(foreach lib,$(EXTLIBS) $(INTLIBS),$(eval include $(lib)/$(lib).mk))
$(foreach prog,$(filter-out eischk,$(PROGS)),$(eval $(call RULE,$(prog))))

$(PREFIX)eischk$(EXEEXT): $(PREFIX)mkfli4l$(EXEEXT)
	@echo "  LD     $@" >&2
	$(CP) $^ $@

archive/squeeze$(SUFFIX).c: LFLAGS := $(LFLAGS) -Psqueeze
archive/squeeze$(SUFFIX).o: CWARN := $(CWARN) -Wno-error=sign-compare

-include $(patsubst %.c,%.P,$(filter %.c,$(SOURCES)))

define MAKEDEPEND
	cp $(1).d $@
	$(SED) -i -e 's,$(1).o *:,$(1).o $@:,' $@
	$(SED) -e 's/#.*//;s/^[^:]*: *//;s/ *\\$$//;/^$$/d;s/$$/:/;s/^ *//' < $(1).d >> $@
	rm -f $(1).d
endef

%_scanner.P: %_scanner.c %_parser.c
	$(CC) $(CFLAGS) $(COPTS) -MM -MG -w -MF $*_scanner.d -MQ $*_scanner.o -c $<
	$(call MAKEDEPEND,$*_scanner)

%_scanner.o: %_scanner.c %_parser.c
	@echo "  CCLEX  $@" >&2
	$(CC) $(CFLAGS) $(COPTS) $(CWARN) -Wno-error -Wno-unused-parameter -Wno-missing-prototypes -o $@ -c $<

%_parser.P: %_parser.c %_scanner.c
	$(CC) $(CFLAGS) $(COPTS) -MM -MG -w -MF $*_parser.d -MQ $*_parser.o -c $<
	$(call MAKEDEPEND,$*_parser)

%_parser.o: %_parser.c %_scanner.c
	@echo "  CCYACC $@" >&2
	$(CC) $(CFLAGS) $(COPTS) $(CWARN) -Wno-error -Wno-unused-parameter -o $@ -c $<

%.P: %.c
	$(CC) $(CFLAGS) $(COPTS) -MM -MG -w -MF $*.d -MQ $*.o -c $<
	$(call MAKEDEPEND,$*)

%.o: %.c
	@echo "  CC     $@" >&2
	$(CC) $(CFLAGS) $(COPTS) $(CWARN) -o $@ -c $<

%_scanner.c %_scanner.h: %_scanner.l
	@echo "  LEX2   $*_scanner.c" >&2
	$(LEX) $(LFLAGS) -P$(notdir $*)_ --header-file=$*_scanner.h -o $*_scanner.c $<

%.c: %.l
	@echo "  LEX    $@" >&2
	$(LEX) $(LFLAGS) -o $@ $<

%-debug.c: %.l
	@echo "  LEX    $@" >&2
	$(LEX) $(LFLAGS) -d -o $@ $<

%_parser.c %_parser.h: %_parser.y
	@echo "  YACC2  $*_parser.c" >&2
	$(YACC) $(YFLAGS) -d -p$(notdir $*)_ -o $*_parser.c $<

%.c %.h: %.y
	@echo "  YACC   $*.c" >&2
	$(YACC) $(YFLAGS) -d -o $*.c $<

%.l: %.ll
	@echo "  LEXPP  $*.l" >&2
	$(CURDIR)/lexyaccpp $@ $<

%.y: %.yy
	@echo "  YACCPP $*.y" >&2
	$(CURDIR)/lexyaccpp $@ $<

grammar/expr_scanner.l: grammar/expr_scanner.rules
grammar/vartype_scanner.l: grammar/expr_scanner.rules
grammar/vardef_scanner.l: grammar/expr_scanner.rules

grammar/expr_parser.y: grammar/expr_parser.rules grammar/expr_parser.tokens
grammar/vartype_parser.y: grammar/expr_parser.rules grammar/expr_parser.tokens
grammar/vardef_parser.y: grammar/expr_parser.rules grammar/expr_parser.tokens

.PHONY: all clean distclean docs install
.SECONDARY: $(common_SOURCES_GENERATED) $(patsubst %.c,%.P,$(filter %.c,$(common_SOURCES_GENERATED)))
