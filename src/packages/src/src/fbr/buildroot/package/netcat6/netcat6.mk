NETCAT6_VERSION  = 1.0
NETCAT6_SOURCE   = nc6-$(NETCAT6_VERSION).tar.bz2
NETCAT6_SITE     = ftp://ftp.deepspace6.net/pub/ds6/sources/nc6
NETCAT6_CONF_OPTS = --disable-nls

NETCAT6_LICENSE = GPLv2+
NETCAT6_LICENSE_FILES = COPYING

define NETCAT6_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/src/nc6 $(TARGET_DIR)/usr/bin/nc6
endef

$(eval $(autotools-package))
