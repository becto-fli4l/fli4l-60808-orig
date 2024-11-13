B2D_VERSION =
B2D_SITE = $(FLI4L_SRCDIR)/wlan
B2D_SITE_METHOD = local
B2D_SOURCE =

define B2D_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) b2d
endef

define B2D_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/b2d $(TARGET_DIR)/sbin/b2d
endef

$(eval $(generic-package))
