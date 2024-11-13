FDPASS_VERSION =
FDPASS_SITE = $(FLI4L_SRCDIR)/base/fdpass
FDPASS_SITE_METHOD = local
FDPASS_SOURCE =
FDPASS_DEPENDENCIES =

define FDPASS_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS)
endef

define FDPASS_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/fdrecv $(TARGET_DIR)/usr/bin/fdrecv
	$(INSTALL) -D $(@D)/fdsend $(TARGET_DIR)/usr/bin/fdsend
endef

$(eval $(generic-package))
