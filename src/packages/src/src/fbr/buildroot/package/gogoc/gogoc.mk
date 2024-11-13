GOGOC_VERSION = 1_2-RELEASE
GOGOC_SITE    = http://content.gogo6.com
GOGOC_SOURCE  = gogoc-$(GOGOC_VERSION).tar.gz

GOGOC_LICENSE = BSD-3c
GOGOC_LICENSE_FILES = CLIENT-LICENSE.TXT

GOGOC_DEPENDENCIES = openssl

define GOGOC_FIX_MAKEFILES
	find $(@D) -name Makefile -exec sed -i \
		-e 's:LDFLAGS:LDLIBS:g' \
		-e '/\$$(LDLIBS)/s,-o,$$(LDFLAGS) -o,' \
		{} +
	sed -i -e 's,/usr/local/etc/gogoc,/etc/gogoc,' \
		$(@D)/gogoc-tsp/platform/*/tsp_local.c
endef
GOGOC_POST_PATCH_HOOKS += GOGOC_FIX_MAKEFILES

define GOGOC_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE1) -C $(@D) $(TARGET_CONFIGURE_OPTS) \
		PLATFORM_CFLAGS="$(TARGET_CFLAGS)" \
		EXTRA_CFLAGS="$(TARGET_CFLAGS)" \
		EXTRA_CXXFLAGS="$(TARGET_CXXFLAGS)" \
		all platform=linux
	+$(TARGET_MAKE_ENV) $(MAKE1) PLATFORM=linux PLATFORM_DIR=../platform \
		BIN_DIR=../bin -C $(@D)/gogoc-tsp/conf gogoc.conf.sample
endef

define GOGOC_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/gogoc-tsp/bin/gogoc $(TARGET_DIR)/usr/sbin/gogoc
	$(INSTALL) -D -m 600 $(@D)/gogoc-tsp/bin/gogoc.conf.sample $(TARGET_DIR)/etc/gogoc/gogoc.conf
	$(INSTALL) -D -m 755 $(@D)/gogoc-tsp/template/linux.sh $(TARGET_DIR)/etc/gogoc/template/linux.sh
endef

$(eval $(generic-package))

