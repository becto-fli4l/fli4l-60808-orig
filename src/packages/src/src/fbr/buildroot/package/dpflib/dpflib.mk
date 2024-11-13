DPFLIB_VERSION = 0.1alpha
DPFLIB_SOURCE = dpfhack-$(DPFLIB_VERSION).tgz
DPFLIB_SITE = http://tech.section5.ch/files

DPFLIB_LICENSE = Proprietary
DPFLIB_LICENSE_FILES = src/README

DPFLIB_INSTALL_STAGING = YES

DPFLIB_DEPENDENCIES = libusb-compat

define DPFLIB_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)/src/dpflib $(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS) -I../include"
endef

define DPFLIB_INSTALL_STAGING_CMDS
	$(INSTALL) -D -m 644 $(@D)/src/dpflib/dpf.h $(STAGING_DIR)/usr/include/libdpf/libdpf.h
	$(INSTALL) -D -m 644 $(@D)/src/dpflib/libdpf.a $(STAGING_DIR)/usr/lib/libdpf.a
endef

define DPFLIB_INSTALL_TARGET_CMDS
endef

$(eval $(generic-package))
