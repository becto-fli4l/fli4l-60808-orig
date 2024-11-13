################################################################################
#
# lua-http
#
################################################################################

LUA_HTTP_VERSION_UPSTREAM = 0.2
LUA_HTTP_VERSION = $(LUA_HTTP_VERSION_UPSTREAM)-0
LUA_HTTP_NAME_UPSTREAM = http
LUA_HTTP_SUBDIR = lua-http-$(LUA_HTTP_VERSION_UPSTREAM)
LUA_HTTP_LICENSE = MIT
LUA_HTTP_LICENSE_FILES = $(LUA_HTTP_SUBDIR)/LICENSE.md
LUA_HTTP_DEPENDENCIES = lua-basexx lua-cqueues lua-fifo lua-lpeg-patterns luaossl

ifeq ($(BR2_PACKAGE_LUA_5_1),y)
LUA_HTTP_DEPENDENCIES += luabitop
endif

$(eval $(luarocks-package))
