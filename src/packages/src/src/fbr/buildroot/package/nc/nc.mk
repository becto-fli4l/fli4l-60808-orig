NC_VERSION  = 1.10
NC_DEBVER   = $(NC_VERSION)-41
NC_SOURCE   = netcat_$(NC_VERSION).orig.tar.bz2
NC_PATCH    = netcat_$(NC_DEBVER).debian.tar.xz
NC_SITE     = http://snapshot.debian.org/archive/debian/20150330T032810Z/pool/main/n/netcat
NC_CONF_OPTS = --disable-nls
NC_CFLAGS   = -DLINUX -DIP_TOS -DDEBIAN_VERSION=\\\"$(NC_DEBVER)\\\"

NC_LICENSE  = Proprietary
NC_LICENSE_FILES = README

define NC_DEBIAN_PATCHES
	$(call suitable-extractor,$(NC_PATCH)) $(DL_DIR)/$(NC_PATCH) | \
		$(TAR) -C $(NC_DIR) $(TAR_OPTIONS) -
	if [ -d $(@D)/debian/patches ]; then \
		support/scripts/apply-patches.sh $(@D) $(@D)/debian/patches; \
	fi
	$(eval NC_PATCH:=)
endef

NC_PRE_PATCH_HOOKS += NC_DEBIAN_PATCHES

define NC_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE1) -C $(@D) CC=$(TARGET_CC) \
		DFLAGS="$(TARGET_CFLAGS) $(NC_CFLAGS)" linux
endef

define NC_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/nc $(TARGET_DIR)/usr/bin/netcat
endef

$(eval $(generic-package))
