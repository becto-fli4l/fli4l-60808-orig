MOM_NETLINK_VERSION =
MOM_NETLINK_SITE = $(FLI4L_SRCDIR)/mom/netlink
MOM_NETLINK_SITE_METHOD = local
MOM_NETLINK_SOURCE =

define MOM_NETLINK_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) mom_netlink
endef

define MOM_NETLINK_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/mom_netlink $(TARGET_DIR)/usr/local/bin/mom_netlink
endef

$(eval $(generic-package))
