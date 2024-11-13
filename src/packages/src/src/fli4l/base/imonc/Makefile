CC      = gcc
CFLAGS  = -O2 -Wall -Wextra
LDFLAGS = -s
LDLIBS  = -lncurses

VERBOSE = @

PROGS = imonc

all: $(PROGS)

clean:
	$(VERBOSE)$(RM) -f *.o *~

cleanall: clean
	$(VERBOSE)$(RM) -f $(PROGS)

imonc: imonc.o
	@echo "  LD $@"
	$(VERBOSE)$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	@echo "  CC $@"
	$(VERBOSE)$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: all clean

# vim: set ft=make:
