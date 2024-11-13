REAVER_WPS_VERSION = 1.4
REAVER_WPS_SOURCE = reaver-$(REAVER_WPS_VERSION).tar.gz
REAVER_WPS_SITE = http://reaver-wps.googlecode.com/files
REAVER_WPS_SUBDIR = src
REAVER_WPS_DEPENDENCIES = sqlite libpcap

REAVER_WPS_LICENSE = GPLv2+
REAVER_WPS_LICENSE_FILES = docs/LICENSE

REAVER_WPS_MAKE = $(MAKE) Q= CC=$(TARGET_CC) AR=$(TARGET_AR) CFLAGS="$(TARGET_CFLAGS)"

define REAVER_WPS_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/src/reaver $(TARGET_DIR)/usr/bin/reaver
endef

$(eval $(autotools-package))
