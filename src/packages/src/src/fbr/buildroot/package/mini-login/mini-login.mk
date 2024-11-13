MINI_LOGIN_VERSION =
MINI_LOGIN_SITE = $(FLI4L_SRCDIR)/base
MINI_LOGIN_SITE_METHOD = local
MINI_LOGIN_SOURCE =

define MINI_LOGIN_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS) -D_GNU_SOURCE" LDLIBS="-lcrypt" \
		mini-login
endef

define MINI_LOGIN_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/mini-login $(TARGET_DIR)/usr/local/bin/mini-login
endef

$(eval $(generic-package))
