DECODE_CHUNKED_VERSION =
DECODE_CHUNKED_SITE = $(FLI4L_SRCDIR)/httpd
DECODE_CHUNKED_SITE_METHOD = local
DECODE_CHUNKED_SOURCE =

define DECODE_CHUNKED_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) decode_chunked
endef

define DECODE_CHUNKED_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/decode_chunked $(TARGET_DIR)/usr/local/bin/decode_chunked
endef

$(eval $(generic-package))
