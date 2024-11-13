# NETSRV_DIR _must_ be set to the directory netsrv lives in!
ifeq ($(NETSRV_DIR),)
$(error NETSRV_DIR is empty, please set it to the directory netsrv lives in)
endif

NETSRV_TARGET=$(NETSRV_DIR)/libnetsrv.a
NETSRV_OBJECTS=$(patsubst %.c,%.o,$(wildcard $(NETSRV_DIR)/src/*.c))
NETSRV_TESTTARGETS=$(patsubst %.c,%,$(wildcard $(NETSRV_DIR)/test/*.c))

CC=gcc
AR=ar
NETSRV_COPTS=-std=gnu89 -I $(NETSRV_DIR)/include -Wall -Wextra -Werror
NETSRV_CFLAGS=-O2
NETSRV_LDFLAGS=-L $(NETSRV_DIR)

ifdef NETSRV_NET_MAX_SERVICES
NETSRV_COPTS+=-DNETSRV_NET_MAX_SERVICES=$(NETSRV_NET_MAX_SERVICES)
endif
ifdef NETSRV_NET_SELECT_TIMEOUT
NETSRV_COPTS+=-DNETSRV_NET_SELECT_TIMEOUT=$(NETSRV_NET_SELECT_TIMEOUT)
endif
ifdef NETSRV_MAX_NESTED_EXCEPTIONS
NETSRV_COPTS+=-DNETSRV_MAX_NESTED_EXCEPTIONS=$(NETSRV_MAX_NESTED_EXCEPTIONS)
endif

# add DEBUG macro if requested
ifeq ($(DEBUG),1)
NETSRV_COPTS += -DDEBUG=1
endif

# rule for building the library
$(NETSRV_TARGET): $(NETSRV_OBJECTS)
	@echo "[AR]   $@" >&2
	$(AR) rcs $@ $^

# rule for building a test program
define TESTRULE
$(patsubst %.c,%,$(1)): $(patsubst %.c,%.o,$(1)) $(NETSRV_TARGET)
	@echo "[CCLD] $$@" >&2
	$(CC) $(NETSRV_COPTS) $(NETSRV_CFLAGS) $(NETSRV_LDFLAGS) -o $$@ $$< -lnetsrv
endef

# generate build rules
$(foreach t,$(wildcard $(NETSRV_DIR)/test/*.c),$(eval $(call TESTRULE,$(t))))

# build object files
$(NETSRV_DIR)/%.o: $(NETSRV_DIR)/%.c
	@echo "[CC]   $@" >&2
	$(CC) $(NETSRV_COPTS) $(NETSRV_CFLAGS) -c -o $@ $<

# run test programs
netsrv-check: $(NETSRV_TESTTARGETS)
	@for t in $(NETSRV_TESTTARGETS); do \
		echo "[RUN]  $$t"; \
		$$t || exit 1; \
	done

# perform clean-up
netsrv-clean:
	rm -f $(NETSRV_OBJECTS)
	rm -f $(NETSRV_TARGET)
	rm -f $(NETSRV_TESTTARGETS)

# build documentation
netsrv-docs:
	doxygen Doxyfile

.PHONY: netsrv-check netsrv-clean netsrv-docs
