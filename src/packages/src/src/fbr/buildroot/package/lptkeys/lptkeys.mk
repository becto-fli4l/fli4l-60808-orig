LPTKEYS_VERSION = 0.0.4
LPTKEYS_SOURCE  = lptkeys-$(LPTKEYS_VERSION).tgz
LPTKEYS_SITE    = http://www.ehnert.net

LPTKEYS_LICENSE = Unknown
LPTKEYS_LICENSE_FILES =

LPTKEYS_DEPENDENCIES = ncurses readline

define LPTKEYS_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) lptkeys
endef

define LPTKEYS_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/lptkeys $(TARGET_DIR)/usr/bin/lptkeys
endef

$(eval $(generic-package))
