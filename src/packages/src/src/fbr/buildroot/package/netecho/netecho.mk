NETECHO_VERSION =
NETECHO_SITE = $(FLI4L_SRCDIR)/netecho
NETECHO_SITE_METHOD = local
NETECHO_SOURCE =

define NETECHO_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) netecho
endef

define NETECHO_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/netecho $(TARGET_DIR)/usr/sbin/netecho
endef

$(eval $(generic-package))
