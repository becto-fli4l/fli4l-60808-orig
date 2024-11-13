PPTPD_VERSION = 1.4.0
PPTPD_SOURCE  = pptpd_$(PPTPD_VERSION).orig.tar.gz
PPTPD_PATCH   = pptpd_$(PPTPD_VERSION)-1.debian.tar.gz
PPTPD_SITE    = http://snapshot.debian.org/archive/debian/20140328T220600Z/pool/main/p/pptpd

PPTPD_LICENSE = GPLv2+
PPTPD_LICENSE_FILES = COPYING

PPTPD_DEPENDENCIES = pppd
PPTPD_CONF_OPTS = --enable-bcrelay
PPTPD_MAKE_OPTS = CC=$(TARGET_CC) LD=$(TARGET_LD) COPTS="$(TARGET_CFLAGS)"

define PPTPD_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/pptpd $(TARGET_DIR)/usr/sbin/pptpd
	$(INSTALL) -D $(@D)/pptpctrl $(TARGET_DIR)/usr/sbin/pptpctrl
	$(INSTALL) -D $(@D)/bcrelay $(TARGET_DIR)/usr/sbin/bcrelay
endef

define PPTPD_DEBIAN_PATCHES
	$(call suitable-extractor,$(PPTPD_PATCH)) $(DL_DIR)/$(PPTPD_PATCH) | \
		$(TAR) -C $(PPTPD_DIR) $(TAR_OPTIONS) -
	if [ -d $(@D)/debian/patches ]; then \
		support/scripts/apply-patches.sh $(@D) $(@D)/debian/patches; \
	fi
	$(eval PPTPD_PATCH:=)
endef

PPTPD_PRE_PATCH_HOOKS += PPTPD_DEBIAN_PATCHES

$(eval $(autotools-package))
