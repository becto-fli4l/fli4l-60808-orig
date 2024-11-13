HWCLOCK_VERSION = 2.45
HWCLOCK_SOURCE  = hwclock-$(HWCLOCK_VERSION).tgz
HWCLOCK_SITE    = ftp://giraffe-data.com/pub/software

HWCLOCK_LICENSE = GPLv2+
HWCLOCK_LICENSE_FILES = GPL_LICENSE.txt

define HWCLOCK_BUILD_CMDS
	echo "HAVE_IO_H = 1" > $(@D)/config.mk
	echo "IO_H_NAMESPACE = sys" >> $(@D)/config.mk
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS)
endef

define HWCLOCK_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/hwclock $(TARGET_DIR)/sbin/hwclock
endef

$(eval $(generic-package))
