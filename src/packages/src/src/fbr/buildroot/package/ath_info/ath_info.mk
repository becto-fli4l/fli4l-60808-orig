ATH_INFO_VERSION     = 4180
ATH_INFO_DL_VERSION  = $(ATH_INFO_VERSION)
ATH_INFO_SITE_METHOD = svn
ATH_INFO_SITE        = http://madwifi-project.org/svn/ath_info/trunk
ATH_INFO_SOURCE      = ath_info-$(ATH_INFO_VERSION).tar.gz

ATH_INFO_LICENSE = GPLv2
ATH_INFO_LICENSE_FILES = 

define ATH_INFO_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) CPPFLAGS=""
endef

define ATH_INFO_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/ath_info $(TARGET_DIR)/usr/bin/ath_info
endef

$(eval $(generic-package))
