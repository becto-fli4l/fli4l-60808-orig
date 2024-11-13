CC      = gcc
CFLAGS  = -O2 -g
COPTS   = -Wall -Wextra -Werror -pedantic
LDFLAGS =
LIBS    =

#***********************************************************************

PROG := ppp2i4l
OBJS := $(patsubst %.c,%.o,$(wildcard *.c))

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
	rm -f $(PROG) $(OBJS)

%.o: %.c
	$(CC) $(COPTS) $(CFLAGS) -c -o $@ $<

.PHONY: all clean
