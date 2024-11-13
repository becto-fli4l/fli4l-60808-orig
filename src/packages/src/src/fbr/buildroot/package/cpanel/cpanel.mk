CPANEL_VERSION =
CPANEL_SITE = $(FLI4L_SRCDIR)/cpanel
CPANEL_SITE_METHOD = local
CPANEL_SOURCE =

define CPANEL_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) cpanel
endef

define CPANEL_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/cpanel $(TARGET_DIR)/usr/local/bin/cpanel
endef

$(eval $(generic-package))
