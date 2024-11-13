MUNINLITE_VERSION = 1.0.4
MUNINLITE_SOURCE = muninlite-$(MUNINLITE_VERSION).tar.gz
MUNINLITE_SITE = http://sourceforge.net/projects/muninlite/files/muninlite%20-%20stable/$(MUNINLITE_VERSION)

MUNINLITE_LICENSE = GPLv2+
MUNINLITE_LICENSE_FILES = LICENSE

MUNINLITE_INSTALL_TARGET = YES

define MUNINLITE_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) all
endef

define MUNINLITE_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/munin-node $(TARGET_DIR)/usr/sbin/munin-node
endef

$(eval $(generic-package))
