STUNTMAN_VERSION = 1.2.13
STUNTMAN_SOURCE  = stunserver-$(STUNTMAN_VERSION).tgz
STUNTMAN_SITE    = http://www.stunprotocol.org

STUNTMAN_DEPENDENCIES = boost openssl

STUNTMAN_LICENSE = Apache
STUNTMAN_LICENSE_FILES = LICENSE

define STUNTMAN_BUILD_CMDS
	+$(HOST_MAKE_ENV) $(MAKE) CXX="$(TARGET_CXX)" LDFLAGS="$(TARGET_LDFLAGS)" -C $(@D)
endef

define STUNTMAN_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/stunclient $(TARGET_DIR)/usr/bin/stunclient
endef

$(eval $(generic-package))
