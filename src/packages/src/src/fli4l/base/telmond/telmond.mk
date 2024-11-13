#----------------------------------------------------------------------------
#  telmond/Makefile
#
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  Creation:       03.05.2000  fm
#  Last Update:    $Id$
#----------------------------------------------------------------------------

PROGS       := $(addprefix $(PREFIX),telmond)
SOURCES     := $(wildcard *.c flicapi/*.c)
EXECUTABLES := $(patsubst %,%$(EXEEXT),$(PROGS))

define TELMOND_RULE
endef

all-telmond: $(EXECUTABLES)

clean-telmond:
	$(VERBOSE)$(RM) -f $(patsubst %.c,%.o,$(SOURCES)) $(patsubst %.c,%.P,$(SOURCES))

cleanall-telmond: clean-telmond
	$(VERBOSE)$(RM) -f $(EXECUTABLES)

$(PREFIX)telmond$(EXEEXT): $(patsubst %.c,%.o,$(SOURCES))
	@echo "  LD    $@"
	$(VERBOSE)$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

-include $(patsubst %.c,%.P,$(SOURCES)))

all: all-telmond
clean: clean-telmond
cleanall: cleanall-telmond

.PHONY: all-telmond clean-telmond cleanall-telmond
