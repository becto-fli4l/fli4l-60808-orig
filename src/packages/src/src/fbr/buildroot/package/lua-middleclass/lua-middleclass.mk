################################################################################
#
# lua-middleclass
#
################################################################################

LUA_MIDDLECLASS_VERSION = 4.1.1
LUA_MIDDLECLASS_SITE = $(call github,kikito,middleclass,v$(LUA_MIDDLECLASS_VERSION))
LUA_MIDDLECLASS_LICENSE = MIT
LUA_MIDDLECLASS_LICENSE_FILES = MIT-LICENSE.txt
LUA_MIDDLECLASS_DEPENDENCIES = luainterpreter

define LUA_MIDDLECLASS_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0644 -D $(@D)/middleclass.lua \
		$(TARGET_DIR)/usr/share/lua/$(LUAINTERPRETER_ABIVER)/middleclass.lua
endef 

$(eval $(generic-package))
