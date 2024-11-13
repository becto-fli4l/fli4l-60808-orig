TARGET=srv868

# default rule
all: $(TARGET)

# parameter for netsrv:
# - allow only two services because that's enough (one TCP and one UDP service)
# - allow two active exceptions at the same time
NETSRV_NET_MAX_SERVICES=2
NETSRV_MAX_NESTED_EXCEPTIONS=2
NETSRV_DIR=netsrv
include $(NETSRV_DIR)/netsrv.mk

CC=gcc
COPTS=-std=gnu89 -I $(NETSRV_DIR)/include -pedantic-errors -Wall -Wextra -Werror
CFLAGS=-O2
override LDFLAGS+=-L $(NETSRV_DIR)

# add DEBUG macro if requested
ifeq ($(DEBUG),1)
COPTS += -DDEBUG=1
endif

OBJECTS=$(patsubst %.c,%.o,$(wildcard src/*.c))

# link target according to fli4l developer documentation
$(TARGET): $(OBJECTS) $(NETSRV_TARGET)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) -lnetsrv

# perform clean-up
clean: netsrv-clean
	rm -f $(OBJECTS)
	rm -f $(TARGET)

# perform tests
check: netsrv-check

# build documentation
docs: netsrv-docs
	doxygen Doxyfile

# build object files
%.o: %.c
	@echo "[CC]   $@" >&2
	$(CC) $(COPTS) $(CFLAGS) -c -o $@ $<

.PHONY: clean check docs
