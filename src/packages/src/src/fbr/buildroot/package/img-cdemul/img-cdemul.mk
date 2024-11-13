IMG_CDEMUL_VERSION =
IMG_CDEMUL_SOURCE  =

IMG_CDEMUL_DEPENDENCIES = host-syslinux host-mtools
IMG_CDEMUL_INSTALL_IMAGES = YES

define IMG_CDEMUL_BUILD_CMDS
	$(HOST_MAKE_ENV) mformat -C -i $(@D)/cdemul.img -f 1440 -v _FLI4L_CD ::
	$(HOST_MAKE_ENV) $(HOST_DIR)/bin/syslinux-cdemul -s $(@D)/cdemul.img
	gzip -f -9 -S.gz $(@D)/cdemul.img
endef

define IMG_CDEMUL_INSTALL_IMAGES_CMDS
	$(INSTALL) -D $(@D)/cdemul.img.gz $(BINARIES_DIR)/cdemul.img.gz
endef

$(eval $(generic-package))
