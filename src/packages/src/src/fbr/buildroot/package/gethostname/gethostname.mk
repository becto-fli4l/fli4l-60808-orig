GETHOSTNAME_VERSION =
GETHOSTNAME_SITE = $(FLI4L_SRCDIR)/gethostname
GETHOSTNAME_SITE_METHOD = local
GETHOSTNAME_SOURCE =

define GETHOSTNAME_BUILD_CMDS
	+$(MAKE) CC=$(TARGET_CC) LD=$(TARGET_LD) CFLAGS="$(TARGET_CFLAGS)" \
		-C $(@D) gethostname
endef

define GETHOSTNAME_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/gethostname $(TARGET_DIR)/usr/local/bin/gethostname
endef

$(eval $(generic-package))
