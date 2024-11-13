#
# Makefile for LCD driver
#

MODULE := lcdhitachi
SOURCES := lcd_module.c
HEADERS := c_table.h lcd.h version.h
OBJECTS := $(patsubst %.c,%.o,$(SOURCES))

ifneq ($(KERNELRELEASE),)

obj-m := $(MODULE).o
$(MODULE)-objs := $(OBJECTS)

else

all: $(MODULE).o

$(MODULE).o: $(SOURCES) $(HEADERS)
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(CURDIR) modules

clean:
	rm -f $(OBJECTS)
	rm -f $(MODULE).o $(MODULE).ko
	@rm -f .*.cmd $(MODULE).mod.*

.PHONY:	clean
endif

install:
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(CURDIR) modules_install

.PHONY:	install
