SFF2MISC_VERSION = 1.0
SFF2MISC_SOURCE  = sff-$(SFF2MISC_VERSION).tar.gz
SFF2MISC_SITE    = http://capircvd.berlios.de/download/sff2misc

SFF2MISC_LICENSE = Unknown
SFF2MISC_LICENSE_FILES =

SFF2MISC_DEPENDENCIES = jpeg

define SFF2MISC_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) \
		LD="$(TARGET_CC)"
endef

define SFF2MISC_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/sff2misc $(TARGET_DIR)/usr/bin/sff2misc
endef

$(eval $(generic-package))
