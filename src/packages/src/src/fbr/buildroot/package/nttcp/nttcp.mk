NTTCP_VERSION = 1.47
NTTCP_SOURCE  = nttcp_$(NTTCP_VERSION).orig.tar.gz
NTTCP_PATCH   = nttcp_$(NTTCP_VERSION)-13.debian.tar.gz
NTTCP_SITE    = http://snapshot.debian.org/archive/debian/20150328T213636Z/pool/non-free/n/nttcp

NTTCP_LICENSE = Public Domain
NTTCP_LICENSE_FILES = 

define NTTCP_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) CC="$(TARGET_CC)" \
		ARCH= \
		OPT="$(TARGET_CFLAGS) -DNOBODY=65534" \
		LFLAGS="$(TARGET_LDFLAGS)"
endef

define NTTCP_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/nttcp $(TARGET_DIR)/usr/bin/nttcp
endef

define NTTCP_DEBIAN_PATCHES
	$(call suitable-extractor,$(NTTCP_PATCH)) $(DL_DIR)/$(NTTCP_PATCH) | \
		$(TAR) -C $(NTTCP_DIR) $(TAR_OPTIONS) -
	if [ -d $(@D)/debian/patches ]; then \
		support/scripts/apply-patches.sh $(@D) $(@D)/debian/patches; \
	fi
	$(eval NTTCP_PATCH:=)
endef

NTTCP_PRE_PATCH_HOOKS += NTTCP_DEBIAN_PATCHES

$(eval $(generic-package))
