IMONC_VERSION =
IMONC_SITE = $(FLI4L_SRCDIR)/base/imonc
IMONC_SITE_METHOD = local
IMONC_SOURCE =
IMONC_DEPENDENCIES = ncurses

define IMONC_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS) -DFLI4L" imonc
endef

define IMONC_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/imonc $(TARGET_DIR)/usr/bin/imonc
endef

$(eval $(generic-package))
