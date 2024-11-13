PCMCIAUTILS_VERSION = 018
PCMCIAUTILS_SOURCE  = pcmciautils_$(PCMCIAUTILS_VERSION).orig.tar.gz
PCMCIAUTILS_PATCH   = pcmciautils_$(PCMCIAUTILS_VERSION)-8.debian.tar.gz
PCMCIAUTILS_SITE    = http://snapshot.debian.org/archive/debian/20141023T043132Z/pool/main/p/pcmciautils

PCMCIAUTILS_LICENSE = GPLv2+
PCMCIAUTILS_LICENSE_FILES = COPYING

PCMCIAUTILS_DEPENDENCIES = host-flex host-bison

define PCMCIAUTILS_DEBIAN_PATCHES
	$(call suitable-extractor,$(PCMCIAUTILS_PATCH)) $(DL_DIR)/$(PCMCIAUTILS_PATCH) | \
		$(TAR) -C $(PCMCIAUTILS_DIR) $(TAR_OPTIONS) -
	if [ -d $(@D)/debian/patches ]; then \
		support/scripts/apply-patches.sh $(@D) $(@D)/debian/patches; \
	fi
	$(eval PCMCIAUTILS_PATCH:=)
endef

PCMCIAUTILS_PRE_PATCH_HOOKS += PCMCIAUTILS_DEBIAN_PATCHES

define PCMCIAUTILS_BUILD_CMDS
	+$(HOST_MAKE_ENV) $(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_CC)" \
		AR="$(TARGET_AR)" RANLIB="$(TARGET_RANLIB)" STRIPCMD=true \
		OPTIMIZATION="$(TARGET_CFLAGS)" LDFLAGS="$(TARGET_LDFLAGS)" \
		UDEV=false DEBUG=false LEX="flex -l" YACC="bison -y" -C $(@D)
endef

define PCMCIAUTILS_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/pccardctl $(TARGET_DIR)/sbin/pccardctl
	ln -sf pccardctl $(TARGET_DIR)/sbin/lspcmcia
	$(INSTALL) -D $(@D)/pcmcia-check-broken-cis $(TARGET_DIR)/usr/sbin/pcmcia-check-broken-cis
	$(INSTALL) -D $(@D)/pcmcia-socket-startup $(TARGET_DIR)/usr/sbin/pcmcia-socket-startup
endef

$(eval $(generic-package))
