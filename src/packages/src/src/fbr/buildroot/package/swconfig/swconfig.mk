SWCONFIG_VERSION = f1bec8cd7215b60491a7dee6b085a1adf2d59609
SWCONFIG_SITE = git://git.openwrt.org/openwrt.git
SWCONFIG_LICENSE = GPLv2

SWCONFIG_DEPENDENCIES = host-pkgconf libuci libnl

SWCONFIG_PKG_PATH = package/network/config/swconfig
SWCONFIG_CFLAGS = $(shell $(PKG_CONFIG_HOST_BINARY) --cflags libnl-3.0 libnl-genl-3.0)
SWCONFIG_LIBS = $(shell $(PKG_CONFIG_HOST_BINARY) --libs libnl-3.0 libnl-genl-3.0) -luci

define SWCONFIG_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)/$(SWCONFIG_PKG_PATH)/src \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS) $(SWCONFIG_CFLAGS)" \
		LIBS="$(SWCONFIG_LIBS)"
endef

define SWCONFIG_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/$(SWCONFIG_PKG_PATH)/src/swconfig \
		$(TARGET_DIR)/usr/sbin/swconfig
	$(INSTALL) -D $(@D)/$(SWCONFIG_PKG_PATH)/src/libsw.so \
		$(TARGET_DIR)/usr/lib/libsw.so
endef

$(eval $(generic-package))
