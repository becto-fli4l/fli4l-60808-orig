KM_FCDSL_X86_64_VERSION = 2.6-37.x86_64
KM_FCDSL_X86_64_SITE    = ftp://ftp.spline.de/pub/gentoo/distfiles
KM_FCDSL_X86_64_SOURCE  = km_fcdsl-$(KM_FCDSL_X86_64_VERSION).rpm
KM_FCDSL_X86_64_LICENSE = Proprietary
KM_FCDSL_X86_64_INSTALL_STAGING = YES

$(eval $(generic-package))

define KM_FCDSL_X86_64_UPDATE_OBJECT_FILES
# fix from Karsten Keil <kkeil@suse.de>
# in refernece to bug: http://bugs.archlinux.org/task/8511
	OBJ=$$(find $(@D)/usr/src/kernel-modules/fcdsl/src/lib -name '*lib.o'); \
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
KM_FCDSL_X86_64_POST_EXTRACT_HOOKS += KM_FCDSL_X86_64_UPDATE_OBJECT_FILES

define KM_FCDSL_X86_64_INSTALL_STAGING_CMDS
	mkdir -p $(STAGING_DIR)/usr/src
	cp -dR $(@D)/usr/src/kernel-modules/fcdsl $(STAGING_DIR)/usr/src/
endef
