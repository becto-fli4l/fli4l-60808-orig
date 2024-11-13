# Makefile for PC-Engines WRAP drivers

ifneq ($(KERNELRELEASE),)

obj-m += leds-wrap.o
obj-m += keys-wrap.o

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
