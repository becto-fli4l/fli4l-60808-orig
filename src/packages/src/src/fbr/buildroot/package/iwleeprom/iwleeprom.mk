#IWLEEPROM_VERSION     =
#IWLEEPROM_SITE_METHOD = local
#IWLEEPROM_SITE        = /home/carsten/workspace/iwleeprom
#IWLEEPROM_SOURCE      =

IWLEEPROM_VERSION     = 43
IWLEEPROM_DL_VERSION  = $(IWLEEPROM_VERSION)
IWLEEPROM_SITE_METHOD = svn
IWLEEPROM_SITE        = https://iwleeprom.googlecode.com/svn/branches/atheros
IWLEEPROM_SOURCE      = iwleeprom-$(IWLEEPROM_VERSION).tar.gz

IWLEEPROM_LICENSE = GPLv2+
IWLEEPROM_LICENSE_FILES =

define IWLEEPROM_BUILD_CMDS
	+$(MAKE1) CC="$(TARGET_CC)" CFLAGS="$(TARGET_CFLAGS)" CPPFLAGS="" -C $(@D)
endef

define IWLEEPROM_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/iwleeprom $(TARGET_DIR)/usr/bin/iwleeprom
endef

$(eval $(generic-package))
