# Makefile for extra gpio drivers

ifneq ($(KERNELRELEASE),)

obj-m += gpio-w83627hf.o

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
