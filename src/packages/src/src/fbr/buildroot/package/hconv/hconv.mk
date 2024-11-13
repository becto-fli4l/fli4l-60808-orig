HCONV_VERSION =
HCONV_SITE = $(FLI4L_SRCDIR)/base
HCONV_SITE_METHOD = local
HCONV_SOURCE =

define HCONV_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) hconv
endef

define HCONV_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/hconv $(TARGET_DIR)/usr/bin/hconv
endef

$(eval $(generic-package))
