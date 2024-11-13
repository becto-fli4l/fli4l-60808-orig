YTREE_VERSION = 1.98
YTREE_SOURCE  = ytree-$(YTREE_VERSION).tar.gz
YTREE_SITE    = http://www.han.de/~werner

YTREE_DEPENDENCIES = ncurses readline

YTREE_LICENSE = GPLv2+
YTREE_LICENSE_FILES = COPYING

define YTREE_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) \
		CC=$(TARGET_CC) ADD_CFLAGS="$(TARGET_CFLAGS)" LFLAGS="$(TARGET_LDFLAGS)"
endef

define YTREE_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/ytree $(TARGET_DIR)/usr/bin/ytree
endef

$(eval $(generic-package))
