PETOOLS_VERSION = 1.0
HOST_PETOOLS_SITE = $(FLI4L_SRCDIR)/petools
HOST_PETOOLS_SITE_METHOD = local
HOST_PETOOLS_SOURCE =

define HOST_PETOOLS_BUILD_CMDS
	+$(HOST_MAKE_ENV) $(MAKE) -C $(@D) $(HOST_CONFIGURE_OPTS)
endef

define HOST_PETOOLS_INSTALL_CMDS
	$(INSTALL) -D $(@D)/pechksum $(HOST_DIR)/bin/pechksum
	$(INSTALL) -D $(@D)/petsset $(HOST_DIR)/bin/petsset
	$(INSTALL) -D $(@D)/petsget $(HOST_DIR)/bin/petsget
endef

$(eval $(host-generic-package))
