K74_VERSION = 20120120
K74_SOURCE  = k74-$(K74_VERSION).tar.bz2
#K74_SITE    = http://quozl.linux.org.au/darcs/k74
K74_SITE    = http://source-archives.nettworks.org

K74_LICENSE = GPLv2+
K74_LICENSE_FILES = COPYING

K74_DEPENDENCIES =

define K74_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) k74-ppdev
endef

define K74_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/k74-ppdev $(TARGET_DIR)/usr/bin/k74-ppdev
endef

$(eval $(generic-package))
