# the package version
TMPL_GENERIC_VERSION = 1.0
# the name of the source archive (without a path!)
TMPL_GENERIC_SOURCE  = tmpl_genric-$(TMPL_GENERIC_VERSION).tar.gz
# the download URL (the name of source archive will be appended automatically!)
TMPL_GENERIC_SITE    = http://www.example.com/tmpl_generic/downloads

# the package dependencies
TMPL_GENERIC_DEPENDENCIES = ncurses host-bison

# the build commands; will be executed after the sources have been downloaded,
# extracted, and patched
define TMPL_GENERIC_BUILD_CMDS
	$(MAKE) CC=$(TARGET_CC) CFLAGS="$(TARGET_CFLAGS)" -C $(@D)
endef

# the installation commands; will be executed after the sources have been
# compiled
define TMPL_GENERIC_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/generic $(TARGET_DIR)/usr/bin/generic
endef

$(eval $(generic-package))
