PCENGINES_VERSION =
PCENGINES_SITE = $(FLI4L_SRCDIR)/pcengines
PCENGINES_SITE_METHOD = local
PCENGINES_SOURCE =

define PCENGINES_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) pcengines
endef

define PCENGINES_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/pcengines $(TARGET_DIR)/usr/sbin/pcengines
endef

$(eval $(generic-package))
