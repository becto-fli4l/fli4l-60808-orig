BYTES2READ_VERSION =
BYTES2READ_SITE = $(FLI4L_SRCDIR)/httpd
BYTES2READ_SITE_METHOD = local
BYTES2READ_SOURCE =

define BYTES2READ_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) bytes2read
endef

define BYTES2READ_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/bytes2read $(TARGET_DIR)/usr/local/bin/bytes2read
endef

$(eval $(generic-package))
