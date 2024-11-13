MINI_HTTPD_VERSION = 1.30
MINI_HTTPD_SOURCE  = mini_httpd-$(MINI_HTTPD_VERSION).tar.gz
MINI_HTTPD_SITE    = http://www.acme.com/software/mini_httpd
MINI_HTTPD_CFLAGS  = -DDISABLE_CONFIG_FILE -D_BSD_SOURCE -D_XOPEN_SOURCE=700

MINI_HTTPD_LICENSE = BSD-2c
MINI_HTTPD_LICENSE_FILES = 

MINI_HTTPD_DEPENDENCIES = openssl

define MINI_HTTPD_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) \
		SSL_LIBS="-lssl -lcrypto" \
		CFLAGS="$(TARGET_CFLAGS) $(MINI_HTTPD_CFLAGS) -DUSE_SSL" \
		mini_httpd
	mv $(@D)/mini_httpd $(@D)/mini_httpd.ssl

	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) clean
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS) $(MINI_HTTPD_CFLAGS)" \
		all
endef

define MINI_HTTPD_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/mini_httpd $(TARGET_DIR)/usr/sbin/mini_httpd
	$(INSTALL) -D $(@D)/mini_httpd.ssl $(TARGET_DIR)/usr/sbin/mini_httpd.ssl
	$(INSTALL) -D $(@D)/htpasswd $(TARGET_DIR)/usr/bin/htpasswd
endef

$(eval $(generic-package))
