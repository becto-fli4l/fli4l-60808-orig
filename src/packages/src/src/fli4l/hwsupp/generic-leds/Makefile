# Makefile for PC-Engines ALIX drivers

ifneq ($(KERNELRELEASE),)

obj-m += leds-simu.o
obj-m += leds-gpio-dyn.o
obj-m += ledtrig-pattern.o

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
