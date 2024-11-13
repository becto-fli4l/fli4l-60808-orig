#
# Toplevel Makefile
# PLEASE look at the subdirectories!
# There are configs to do! Especially in driver/
#
ifeq ($(CC),)
CC=gcc
endif

export KERNELSRC CC

all:
	make -C driver
	make -C driver-rr
	make -C tools/proclcd
	make -C tools/graphics
	make -C tools/lcdtime

install:
	make -C driver install
	make -C driver-rr install

clean:
	make -C driver clean
	make -C driver-rr clean
	make -C tools/proclcd clean
	make -C tools/graphics clean
	make -C tools/lcdtime clean


