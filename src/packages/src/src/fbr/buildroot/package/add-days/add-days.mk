ADD_DAYS_VERSION =
ADD_DAYS_SITE = $(FLI4L_SRCDIR)/base
ADD_DAYS_SITE_METHOD = local
ADD_DAYS_SOURCE =

define ADD_DAYS_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) add-days
endef

define ADD_DAYS_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/add-days $(TARGET_DIR)/usr/local/bin/add-days
endef

$(eval $(generic-package))
