# Makefile for PC-Engines APU drivers

ifneq ($(KERNELRELEASE),)

obj-m += gpio-apu.o
obj-m += gpio-apu2.o
obj-m += leds-apu.o
obj-m += keys-apu.o
obj-m += gpio-nct5104d.o
obj-m += fch16h3x-wdt.o

else

.PHONY:	all modules install clean
all: modules

modules:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) modules

install:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) modules_install

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) clean

endif
