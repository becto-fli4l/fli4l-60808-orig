################################################################################
#
# concurrentlua
#
################################################################################

CONCURRENTLUA_VERSION = 1.1
CONCURRENTLUA_SITE = $(call github,lefcha,concurrentlua,v$(CONCURRENTLUA_VERSION))
CONCURRENTLUA_LICENSE = MIT
CONCURRENTLUA_LICENSE_FILES = LICENSE
CONCURRENTLUA_DEPENDENCIES = luainterpreter copas

define CONCURRENTLUA_BUILD_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) MYCFLAGS="$(TARGET_CFLAGS)" \
		MYLDFLAGS="$(TARGET_LDFLAGS)" -C $(@D) all
endef

define CONCURRENTLUA_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) \
		CC="$(TARGET_CC)" LD="$(TARGET_LD)" \
		DESTDIR="$(TARGET_DIR)" PREFIX="/usr" \
		LUAVERSION="$(LUAINTERPRETER_ABIVER)" -C $(@D) install
endef

$(eval $(generic-package))
