ISAPNPTOOLS_VERSION = 1.27
ISAPNPTOOLS_SOURCE  = isapnptools-$(ISAPNPTOOLS_VERSION).tgz
ISAPNPTOOLS_SITE    = ftp://metalab.unc.edu/pub/Linux/system/hardware

ISAPNPTOOLS_LICENSE = GPLv2+
ISAPNPTOOLS_LICENSE_FILES = COPYING

define ISAPNPTOOLS_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/src/isapnp $(TARGET_DIR)/sbin/isapnp
	$(INSTALL) -D $(@D)/src/pnpdump $(TARGET_DIR)/sbin/pnpdump
endef

$(eval $(autotools-package))
