PROCCGI_SOURCE = proccgi.c
PROCCGI_SITE   = http://www.fpx.de/fp/Software
PROCCGI_EXTRACT_CMDS = cp -dp $(DL_DIR)/$(PROCCGI_SOURCE) $(PROCCGI_DIR)

PROCCGI_LICENSE = GPL
PROCCGI_LICENSE_FILES =

define PROCCGI_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) proccgi
endef

define PROCCGI_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/proccgi $(TARGET_DIR)/usr/local/bin/proccgi
endef

$(eval $(generic-package))
