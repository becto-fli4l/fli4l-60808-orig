HORST_VERSION = 4.2
HORST_SOURCE  = horst-$(HORST_VERSION).tar.gz
HORST_SITE    = http://br1.einfach.org/horst_dl

HORST_LICENSE = GPLv2+
HORST_LICENSE_FILES = LICENSE

HORST_DEPENDENCIES = ncurses libpcap

define HORST_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) horst
endef

define HORST_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/horst $(TARGET_DIR)/usr/sbin/horst
endef

$(eval $(generic-package))
