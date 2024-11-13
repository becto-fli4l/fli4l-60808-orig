#############################################################
#
# rtpengine
#
#############################################################

RTPENGINE_VERSION = 3.5.1.2+0~mr3.5.1.2
RTPENGINE_SOURCE  = ngcp-rtpengine_$(RTPENGINE_VERSION).tar.gz
RTPENGINE_SITE    = http://deb.sipwise.com/spce/mr3.5.1/pool/main/n/ngcp-rtpengine

RTPENGINE_LICENSE = GPLv3
RTPENGINE_LICENSE_FILES = LICENSE

RTPENGINE_DEPENDENCIES = libglib2 pcre libxmlrpc

RTPENGINE_MAKE_ENV = \
	PKG_CONFIG_SYSROOT_DIR="$(STAGING_DIR)" \
	PKG_CONFIG_PATH="$(STAGING_DIR)/usr/lib/pkgconfig"

define RTPENGINE_BUILD_CMDS
	+$(HOST_MAKE_ENV) $(RTPENGINE_MAKE_ENV) $(MAKE) -C $(@D)/daemon CC=$(TARGET_CC) DBG= rtpengine
endef

define RTPENGINE_FIX_MAKEFILE
        $(SED) 's#^CFLAGS=\(.*\)#CFLAGS=\1 $(TARGET_CFLAGS)#' $(@D)/daemon/Makefile
        $(SED) 's#-DMEDIAPROXY_VERSION.*$$#-DMEDIAPROXY_VERSION="\\"$(RTPENGINE_VERSION)\\""#' $(@D)/daemon/Makefile
	$(SED) 's#`xmlrpc-c-config client --libs`#-lxmlrpc_client -lxmlrpc -lxmlrpc_util -lxmlrpc_xmlparse -lxmlrpc_xmltok -lcurl#' $(@D)/daemon/Makefile
        $(SED) 's#`pcre-config --cflags`##' $(@D)/daemon/Makefile
        $(SED) 's#`pcre-config --libs`#-lpcre#g' $(@D)/daemon/Makefile
        $(SED) 's#ifneq ($$(DBG),yes)#ifeq (a,b)#' $(@D)/daemon/Makefile
        $(SED) 's#-I/lib/modules/`uname -r`/build/include/##' $(@D)/daemon/Makefile
        $(SED) 's#CFLAGS+=	-O3##' $(@D)/daemon/Makefile
endef
RTPENGINE_POST_EXTRACT_HOOKS += RTPENGINE_FIX_MAKEFILE

define RTPENGINE_INSTALL_TARGET_CMDS
        $(INSTALL) -D $(@D)/daemon/rtpengine $(TARGET_DIR)/usr/bin/rtpengine
endef

define RTPENGINE_UNINSTALL_TARGET_CMDS
	-rm $(TARGET_DIR)/usr/bin/rtpengine
endef

define RTPENGINE_CLEAN_CMDS
	+-$(MAKE) -C $(@D)/daemon clean
endef

$(eval $(generic-package))
