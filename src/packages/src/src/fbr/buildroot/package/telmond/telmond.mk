TELMOND_VERSION =
TELMOND_SITE = $(FLI4L_SRCDIR)/base/telmond
TELMOND_SITE_METHOD = local
TELMOND_SOURCE =

TELMOND_DEPENDENCIES = libcapi20

define TELMOND_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) ARCH=$(ARCH)
endef

define TELMOND_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/$(ARCH)-linux-telmond $(TARGET_DIR)/usr/local/bin/telmond
endef

$(eval $(generic-package))
