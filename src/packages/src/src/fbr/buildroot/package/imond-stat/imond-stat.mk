IMOND_STAT_VERSION =
IMOND_STAT_SITE = $(FLI4L_SRCDIR)/base/imond
IMOND_STAT_SITE_METHOD = local
IMOND_STAT_SOURCE =

define IMOND_STAT_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) imond-stat
endef

define IMOND_STAT_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/imond-stat $(TARGET_DIR)/usr/local/bin/imond-stat
endef

$(eval $(generic-package))
