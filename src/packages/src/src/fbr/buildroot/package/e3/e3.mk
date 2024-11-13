E3_VERSION = 2.82
E3_SOURCE  = e3-$(E3_VERSION).tgz
E3_SITE    = http://sites.google.com/site/e3editor/Home

E3_LICENSE = GPLv2+
E3_LICENSE_FILES = COPYING.GPL

E3_DEPENDENCIES = host-nasm

ifeq ($(BR2_ARCH_IS_64),y)
E3_TARGET = 64
else
E3_TARGET = 32
endif

define E3_BUILD_CMDS
	+$(HOST_MAKE_ENV) $(MAKE) LD="$(TARGET_LD)" \
		LDFLAGS="$(patsubst -Wl$(comma)%,%,$(TARGET_LDFLAGS))" \
		-C $(@D) $(E3_TARGET)
endef

define E3_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/e3 $(TARGET_DIR)/usr/bin/e3
endef

$(eval $(generic-package))
