ISDN_RATE_VERSION =
ISDN_RATE_SITE = $(FLI4L_SRCDIR)/lcd/isdn_rate
ISDN_RATE_SITE_METHOD = local
ISDN_RATE_SOURCE =

define ISDN_RATE_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) isdn_rate
endef

define ISDN_RATE_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/isdn_rate $(TARGET_DIR)/usr/local/bin/isdn_rate
endef

$(eval $(generic-package))
