################################################################################
#
# lua-cqueues
#
################################################################################

LUA_CQUEUES_VERSION_UPSTREAM = 20161215
LUA_CQUEUES_VERSION = $(LUA_CQUEUES_VERSION_UPSTREAM).53-0
LUA_CQUEUES_NAME_UPSTREAM = cqueues
LUA_CQUEUES_SUBDIR = cqueues-rel-$(LUA_CQUEUES_VERSION_UPSTREAM)
LUA_CQUEUES_LICENSE = MIT
LUA_CQUEUES_LICENSE_FILES = $(LUA_CQUEUES_SUBDIR)/LICENSE
LUA_CQUEUES_DEPENDENCIES = luainterpreter openssl host-m4

$(eval $(luarocks-package))

LUA_CQUEUES_VAR_PREFIX = LUA$(subst .,,$(LUAINTERPRETER_ABIVER))
$(LUA_CQUEUES_TARGET_INSTALL_TARGET_SANDBOX): LUAROCKS_RUN_ENV += M4=$(HOST_DIR)/bin/m4 \
	LUA_APIS=$(LUAINTERPRETER_ABIVER) \
	$(LUA_CQUEUES_VAR_PREFIX)_CPPFLAGS="" \
	$(LUA_CQUEUES_VAR_PREFIX)C=true
