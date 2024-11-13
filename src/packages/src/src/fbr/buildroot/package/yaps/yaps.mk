YAPS_VERSION = 0.96
YAPS_SOURCE  = yaps-$(YAPS_VERSION).tar.gz
YAPS_SITE    = ftp://metalab.unc.edu/pub/Linux/apps/serialcomm/machines

YAPS_LICENSE = GPL
YAPS_LICENSE_FILES = COPYING.GPL \
                     README

define YAPS_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS) -D_POSIX_C_SOURCE=200112L"
endef

define YAPS_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/yaps $(TARGET_DIR)/usr/bin/yaps
endef

$(eval $(generic-package))
