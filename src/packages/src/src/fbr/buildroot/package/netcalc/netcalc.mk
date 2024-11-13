NETCALC_VERSION =
NETCALC_SITE = $(FLI4L_SRCDIR)/base
NETCALC_SITE_METHOD = local
NETCALC_SOURCE =

define NETCALC_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) netcalc
endef

define NETCALC_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/netcalc $(TARGET_DIR)/usr/bin/netcalc
endef

$(eval $(generic-package))
