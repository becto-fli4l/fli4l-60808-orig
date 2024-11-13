KM_FRITZCAPI_VERSION = 2.6-43.i586
KM_FRITZCAPI_SITE    = ftp://ftp.spline.de/pub/gentoo/distfiles
KM_FRITZCAPI_SOURCE  = km_fritzcapi-$(KM_FRITZCAPI_VERSION).rpm
KM_FRITZCAPI_LICENSE = Proprietary
KM_FRITZCAPI_INSTALL_STAGING = YES

define KM_FRITZCAPI_UPDATE_OBJECT_FILES
# fix from Karsten Keil <kkeil@suse.de>
# in refernece to bug: http://bugs.archlinux.org/task/8511
	OBJ=$$(find $(@D)/usr/src/kernel-modules/fritzcapi/*/lib -name '*lib.o'); \
	for i in $$OBJ; do \
		mv $$i $$i.old; \
		objcopy -L memcmp \
			-L memcpy \
			-L memmove \
			-L memset \
			-L strcat \
			-L strcmp \
			-L strcpy \
			-L strlen \
			-L strncmp \
			-L strncpy \
			$$i.old $$i; \
		rm -f $$i.old; \
	done
endef
KM_FRITZCAPI_POST_EXTRACT_HOOKS += KM_FRITZCAPI_UPDATE_OBJECT_FILES

define KM_FRITZCAPI_INSTALL_STAGING_CMDS
	mkdir -p $(STAGING_DIR)/usr/src
	cp -dR $(@D)/usr/src/kernel-modules/fritzcapi $(STAGING_DIR)/usr/src/
endef

$(eval $(generic-package))
