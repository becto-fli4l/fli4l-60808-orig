LPDSRV_VERSION = 1.4
LPDSRV_SOURCE  = lpdsrv.tgz
LPDSRV_SITE    = http://www.ledow.org.uk/Freesco/Programs

LPDSRV_LICENSE = GPL
LPDSRV_LICENESE_FILES =

LPDSRV_DEPENDENCIES =

LPDSRV_EXTRACT_CMDS = $(INFLATE.tgz) $(DL_DIR)/$(LPDSRV_SOURCE) \
	| $(TAR) -C $(LPDSRV_DIR) $(TAR_OPTIONS) -

define LPDSRV_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) LDLIBS=""
endef

define LPDSRV_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/lpdsrv $(TARGET_DIR)/usr/sbin/lpdsrv
endef

$(eval $(generic-package))
