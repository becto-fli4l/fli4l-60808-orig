CRON_VERSION = 3.0pl1
CRON_SOURCE  = cron_$(CRON_VERSION).orig.tar.gz
CRON_PATCH   = cron_$(CRON_VERSION)-124.2.diff.gz
CRON_SITE    = http://snapshot.debian.org/archive/debian/20141023T043132Z/pool/main/c/cron

CRON_LICENSE = Proprietary
CRON_LICENSE_FILES = README

define CRON_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) CC="$(TARGET_CC)" \
		OPTIM="$(TARGET_CFLAGS)" LDFLAGS="$(TARGET_LDFLAGS)"
endef

define CRON_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/cron $(TARGET_DIR)/usr/sbin/cron
endef

$(eval $(generic-package))
