LOCKCOMMAND_VERSION =
LOCKCOMMAND_SITE = $(FLI4L_SRCDIR)/lockcommand
LOCKCOMMAND_SITE_METHOD = local
LOCKCOMMAND_SOURCE =

define LOCKCOMMAND_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) lockcommand
endef

define LOCKCOMMAND_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/lockcommand $(TARGET_DIR)/sbin/lockcommand
endef

$(eval $(generic-package))
