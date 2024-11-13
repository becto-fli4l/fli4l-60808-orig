UCLIBCPP_MAJOR_VERSION = 0
UCLIBCPP_VERSION       = $(UCLIBCPP_MAJOR_VERSION).2.4
UCLIBCPP_SOURCE        = uClibc++-$(UCLIBCPP_VERSION).tar.xz
UCLIBCPP_SITE          = http://cxx.uclibc.org/src

UCLIBCPP_LICENSE = LGPLv2.1+, GPLv2+
UCLIBCPP_LICENSE_FILES = 

UCLIBCPP_INSTALL_STAGING = YES

UCLIBCPP_DEPENDENCIES =

define UCLIBCPP_ADD_CONFIG
	cp -a package/uclibcpp/dot-config.$(FBR_ARCH) $(UCLIBCPP_DIR)/.config
endef
UCLIBCPP_POST_EXTRACT_HOOKS += UCLIBCPP_ADD_CONFIG

define UCLIBCPP_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) CC=$(TARGET_CC) CXX=$(TARGET_CXX) CPU_CFLAGS="$(TARGET_CFLAGS)" \
		OPTIMIZATION= LDFLAGS="$(TARGET_LDFLAGS)" STRIPTOOL=true
endef

define UCLIBCPP_INSTALL_STAGING_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)/include PREFIX=$(STAGING_DIR)/usr/include/uClibc++ UCLIBCXX_RUNTIME_INCLUDEDIR= install
	$(INSTALL) -D $(@D)/src/libuClibc++-$(UCLIBCPP_VERSION).so $(STAGING_DIR)/usr/lib/libuClibc++.so.$(UCLIBCPP_VERSION)
	ln -s libuClibc++.so.$(UCLIBCPP_VERSION) $(STAGING_DIR)/usr/lib/libuClibc++.so.$(UCLIBCPP_MAJOR_VERSION)
	ln -s libuClibc++.so.$(UCLIBCPP_MAJOR_VERSION) $(STAGING_DIR)/usr/lib/libuClibc++.so
endef

define UCLIBCPP_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/src/libuClibc++-$(UCLIBCPP_VERSION).so $(TARGET_DIR)/usr/lib/libuClibc++.so.$(UCLIBCPP_VERSION)
	ln -s libuClibc++.so.$(UCLIBCPP_VERSION) $(TARGET_DIR)/usr/lib/libuClibc++.so.$(UCLIBCPP_MAJOR_VERSION)
	ln -s libuClibc++.so.$(UCLIBCPP_MAJOR_VERSION) $(TARGET_DIR)/usr/lib/libuClibc++.so
endef

$(eval $(generic-package))
