WATCHDOGD_VERSION =
WATCHDOGD_SITE = $(FLI4L_SRCDIR)/watchdog
WATCHDOGD_SITE_METHOD = local
WATCHDOGD_SOURCE =

define WATCHDOGD_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) watchdogd
endef

define WATCHDOGD_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/watchdogd $(TARGET_DIR)/usr/sbin/watchdogd
endef

$(eval $(generic-package))
