MAKEFILE_LIST := $(patsubst $(lastword $(MAKEFILE_LIST)),,$(MAKEFILE_LIST))
AVMFRITZ := $(call UPPERCASE,$(pkgname))

ifeq (y,$(BR2_ARCH_IS_64))
$(AVMFRITZ)_AVMTYPE := $($(AVMFRITZ)_AVMTYPE)_x86_64
endif

$(AVMFRITZ)_VERSION      := $($(call UPPERCASE,$($(AVMFRITZ)_AVMTYPE))_VERSION)-$(call qstrip,$(BR2_LINUX_KERNEL_VERSION))
$(AVMFRITZ)_SITE          =
$(AVMFRITZ)_SOURCE        =
$(AVMFRITZ)_DEPENDENCIES := $($(AVMFRITZ)_AVMTYPE)
$(AVMFRITZ)_LICENSE       = Proprietary

$(AVMFRITZ)_MODULE_SUBDIRS  := fritz/src
$(AVMFRITZ)_MODULE_MAKE_OPTS = \
	CARD=$($(AVMFRITZ)_CARD) \
	KDIR=$($(AVMFRITZ)_DIR) \
	LIBDIR=$(@D)/fritz/lib

define $(AVMFRITZ)_CONFIGURE_CMDS
	mkdir -p $(@D)/fritz
	cp -dR $(STAGING_DIR)/usr/src/$($(PKG)_SUBDIR_DRIVER) $(@D)/fritz/src
	cp -dR $(STAGING_DIR)/usr/src/$($(PKG)_SUBDIR_LIB) $(@D)/fritz/lib
endef

$(eval $(kernel-module))
$(eval $(call generic-package,$($(AVMFRITZ)_CARD)))

# needed because $(AVMFRITZ)_MODULE_MAKE_OPTS references $(AVMFRITZ),
# so we have to store its value per package target
$($(AVMFRITZ)_TARGET_BUILD): AVMFRITZ := $(AVMFRITZ)
$($(AVMFRITZ)_TARGET_INSTALL_TARGET_SANDBOX): AVMFRITZ := $(AVMFRITZ)
