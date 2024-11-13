BASEDIR = ../../../../../base
INSTALLDIR = ../../../../opt/files/lib/modules/`cat $(BASEDIR)/opt/etc/kernel-version`/misc

ifeq ($(KERNELSRC),)
foo = $(error please set KERNELSRC)
endif

#
# If you want some debugging output then add -DDEBUG to the CFLAGS
# Attention: This will fill your syslog quite fast!
#
CFLAGS  =       $(foo) -D__KERNEL__ -I$(KERNELSRC)/include -Wall \
                -Wstrict-prototypes -Os -fomit-frame-pointer -pipe \
                -ffixed-8 -DMODULE 

lcd-winamp.o:	lcd_module.o
	cp $< $@

lcd_module.o:	lcd_module.c c_table.h

.PHONY:	install clean

install: lcd-winamp.o
	[ -d $(INSTALLDIR) ] || mkdir -p $(INSTALLDIR)
	cp $^ $(INSTALLDIR)

clean:
	rm -f *.o
