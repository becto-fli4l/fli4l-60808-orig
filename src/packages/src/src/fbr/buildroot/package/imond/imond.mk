IMOND_VERSION =
IMOND_SITE = $(FLI4L_SRCDIR)/base/imond
IMOND_SITE_METHOD = local
IMOND_SOURCE =

define IMOND_BUILD_CMDS
	+echo "imond: imond.o md5.o" | \
		$(TARGET_MAKE_ENV) $(MAKE) -f - -C $(@D) $(TARGET_CONFIGURE_OPTS) \
			LDLIBS="-lcrypt" imond
endef

define IMOND_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/imond $(TARGET_DIR)/usr/local/bin/imond
endef

$(eval $(generic-package))
