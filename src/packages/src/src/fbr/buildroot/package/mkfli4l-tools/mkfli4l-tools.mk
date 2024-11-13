MKFLI4L_TOOLS_VERSION =
MKFLI4L_TOOLS_SITE = $(FLI4L_SRCDIR)/base/mkfli4l-tools
MKFLI4L_TOOLS_SITE_METHOD = local
MKFLI4L_TOOLS_SOURCE =

MKFLI4L_TOOLS_DEPENDENCIES = host-flex host-bison libiconv

define MKFLI4L_TOOLS_BUILD_CMDS
	+$(HOST_MAKE_ENV) $(MAKE) CROSS=$(GNU_TARGET_NAME)- \
		CHOST=$(ARCH)-pc-linux-gnu CTARGET=$(GNU_TARGET_NAME) \
		ARCH= TUNE=$(BR2_GCC_TARGET_TUNE) OS=linux PREFIX= \
		CFLAGS="$(TARGET_CFLAGS)" LDFLAGS="$(TARGET_LDFLAGS)" STATIC=0 \
		COVERAGE=0 DEBUG=0 STRIP= VERBOSE=0 RUNTIME=uclibc -C $(@D)
endef

define MKFLI4L_TOOLS_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/mkfli4l $(TARGET_DIR)/usr/local/bin/mkfli4l
	$(INSTALL) -D $(@D)/regexp $(TARGET_DIR)/usr/local/bin/regexp
	$(INSTALL) -D $(@D)/squeeze $(TARGET_DIR)/usr/local/bin/squeeze
endef

$(eval $(generic-package))
