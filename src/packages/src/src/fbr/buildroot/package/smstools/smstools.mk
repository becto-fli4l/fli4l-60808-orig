# Do not to be need updated, as new version is 4-5 time bigger !
SMSTOOLS_VERSION = 2.2.20
SMSTOOLS_SOURCE  = smstools-$(SMSTOOLS_VERSION).tar.gz
SMSTOOLS_SITE    = http://meinemullemaus.de/smstools

SMSTOOLS_LICENSE = GPL
SMSTOOLS_LICENSE_FILES = doc/license.html \
                         doc/gpl_de.html \
                         doc/gpl.html

SMSTOOLS_DEPENDENCIES =

define SMSTOOLS_BUILD_CMDS
	+$(MAKE) CC=$(TARGET_CC) CFLAGS="$(TARGET_CFLAGS)" -C $(@D)
endef

define SMSTOOLS_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/src/smsd $(TARGET_DIR)/usr/sbin/smsd
	$(INSTALL) -D $(@D)/scripts/sendsms $(TARGET_DIR)/usr/bin/sendsms
endef

$(eval $(generic-package))
