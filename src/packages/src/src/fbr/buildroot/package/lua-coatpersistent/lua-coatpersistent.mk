################################################################################
#
# lua-coatpersistent
#
################################################################################

LUA_COATPERSISTENT_VERSION_UPSTREAM = 0.2.2
ifeq ($(BR2_PACKAGE_LSQLITE3),y)
LUA_COATPERSISTENT_VERSION = lsqlite3-$(LUA_COATPERSISTENT_VERSION_UPSTREAM)-1
else
LUA_COATPERSISTENT_VERSION = luasql-$(LUA_COATPERSISTENT_VERSION_UPSTREAM)-1
endif
LUA_COATPERSISTENT_SUBDIR = lua-CoatPersistent-$(LUA_COATPERSISTENT_VERSION_UPSTREAM)
LUA_COATPERSISTENT_LICENSE = MIT
LUA_COATPERSISTENT_LICENSE_FILES = $(LUA_COATPERSISTENT_SUBDIR)/COPYRIGHT
LUA_COATPERSISTENT_DEPENDENCIES = dado lua-coat

ifeq ($(BR2_PACKAGE_LSQLITE3),y)
LUA_COATPERSISTENT_DEPENDENCIES += lsqlite3
endif

ifeq ($(BR2_PACKAGE_LUASQL_SQLITE3),y)
LUA_COATPERSISTENT_DEPENDENCIES += luasql-sqlite3
endif

$(eval $(luarocks-package))
