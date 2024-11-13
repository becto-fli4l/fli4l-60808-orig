################################################################################
#
# improxy
#
################################################################################

IMPROXY_VERSION = 5a9d153d10e19ef835af9fc5f2a971799677a7ed
IMPROXY_SITE = https://github.com/haibbo/improxy.git
IMPROXY_SITE_METHOD = git

define IMPROXY_BUILD_CMDS
    $(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D)
endef

define IMPROXY_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m 0755 $(@D)/improxy $(TARGET_DIR)/usr/sbin
endef

$(eval $(generic-package))
