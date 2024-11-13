OVPNCTRL_VERSION =
OVPNCTRL_SITE = $(FLI4L_SRCDIR)/vpn
OVPNCTRL_SITE_METHOD = local
OVPNCTRL_SOURCE = 

define OVPNCTRL_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) ovpnctrl
endef

define OVPNCTRL_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/ovpnctrl $(TARGET_DIR)/usr/bin/ovpnctrl
endef

$(eval $(generic-package))
