GOBI_LOADER_VERSION = 0.7
GOBI_LOADER_SITE = http://www.codon.org.uk/~mjg59/gobi_loader/download

GOBI_LOADER_LICENSE = GPLv2+
GOBI_LOADER_LICENSE_FILES = README

define GOBI_LOADER_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS)
endef

define GOBI_LOADER_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/gobi_loader $(TARGET_DIR)/usr/bin/gobi_loader
endef

$(eval $(generic-package))
