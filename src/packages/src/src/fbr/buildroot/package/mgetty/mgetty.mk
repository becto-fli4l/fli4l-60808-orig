MGETTY_VERSION = 1.1.37-Jun05
MGETTY_SOURCE  = mgetty$(MGETTY_VERSION).tar.gz
MGETTY_SITE    = ftp://mgetty.greenie.net/pub/mgetty/source/1.1

MGETTY_LICENSE = GPLv2+
MGETTY_LICENSE_FILES = COPYING \
                       README.1st

MGETTY_DEPENDENCIES =

define MGETTY_COPY_POLICY
	cp $(@D)/policy.h-dist $(@D)/policy.h
endef
MGETTY_POST_PATCH_HOOKS += MGETTY_COPY_POLICY

define MGETTY_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) mgetty
endef

define MGETTY_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/mgetty $(TARGET_DIR)/usr/sbin/mgetty
endef

$(eval $(generic-package))
