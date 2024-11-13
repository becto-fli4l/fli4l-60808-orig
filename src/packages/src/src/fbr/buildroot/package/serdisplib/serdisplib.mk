SERDISPLIB_MAJOR_VERSION = 2
SERDISPLIB_VERSION = $(SERDISPLIB_MAJOR_VERSION).01
SERDISPLIB_SOURCE = serdisplib-$(SERDISPLIB_VERSION).tar.gz
SERDISPLIB_SITE = http://downloads.sourceforge.net/project/serdisplib/serdisplib/$(SERDISPLIB_VERSION)
SERDISPLIB_AUTORECONF = YES
SERDISPLIB_INSTALL_STAGING = YES

SERDISPLIB_LICENSE = GPLv2+
SERDISPLIB_LICENSE_FILES = COPYING

SERDISPLIB_DEPENDENCIES = libusb-compat gd

SERDISPLIB_CONF_ENV = LIBUSB_CONFIG=$(STAGING_DIR)/usr/bin/libusb-config
SERDISPLIB_CONF_OPTS = --enable-libusb --disable-statictools --disable-libSDL
SERDISPLIB_MAKE_OPTS = STATIC_LDFLAGS="$(TARGET_LDFLAGS)"

# serdisplib's 'install' target does not respect DESTDIR!
define SERDISPLIB_INSTALL_STAGING_CMDS
	for f in $(@D)/include/serdisplib/*.h; do \
		$(INSTALL) -D -m 644 $$f $(STAGING_DIR)/usr/include/serdisplib/$$(basename $$f); \
	done
	$(INSTALL) -D $(@D)/lib/libserdisp.so.$(SERDISPLIB_VERSION) $(STAGING_DIR)/usr/lib/libserdisp.so.$(SERDISPLIB_VERSION)
	ln -sf libserdisp.so.$(SERDISPLIB_VERSION) $(STAGING_DIR)/usr/lib/libserdisp.so
endef

# serdisplib's 'install' target does not respect DESTDIR!
define SERDISPLIB_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/lib/libserdisp.so.$(SERDISPLIB_VERSION) $(TARGET_DIR)/usr/lib/libserdisp.so.$(SERDISPLIB_VERSION)
	ln -sf libserdisp.so.$(SERDISPLIB_VERSION) $(TARGET_DIR)/usr/lib/libserdisp.so.$(SERDISPLIB_MAJOR_VERSION)
	$(INSTALL) -D $(@D)/src/testserdisp $(TARGET_DIR)/usr/bin/testserdisp
	$(INSTALL) -D $(@D)/tools/l4m132c_tool $(TARGET_DIR)/usr/bin/l4m132c_tool
	$(INSTALL) -D $(@D)/tools/multidisplay $(TARGET_DIR)/usr/bin/multidisplay
endef

$(eval $(autotools-package))
