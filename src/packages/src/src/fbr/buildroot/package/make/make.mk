################################################################################
#
# make
#
################################################################################

MAKE_VERSION = 4.3
MAKE_SOURCE = make-$(MAKE_VERSION).tar.gz
MAKE_SITE = $(BR2_GNU_MIRROR)/make
MAKE_DEPENDENCIES = $(TARGET_NLS_DEPENDENCIES) host-pkgconf
MAKE_LICENSE = GPL-3.0+
MAKE_LICENSE_FILES = COPYING
MAKE_CONF_OPTS = --without-guile
# Patching configure.ac
# MAKE_AUTORECONF = YES
# MAKE V4.3 need automake > 1.16.1, but is not available at the moment in old FBR

HOST_MAKE_DEPENDENCIES = host-pkgconf

define HOST_MAKE_PATCH_LIBDIR
	sed -ie 's,@LIBDIR@,$(STAGING_DIR)/lib,;s,@USRLIBDIR@,$(STAGING_DIR)/usr/lib,' $(@D)/src/remake.c
endef

HOST_MAKE_POST_PATCH_HOOKS += HOST_MAKE_PATCH_LIBDIR

define MAKE_RESTORE_LIBDIR
	sed -ie 's,@LIBDIR@,/lib,;s,@USRLIBDIR@,/usr/lib,' $(@D)/src/remake.c
endef

MAKE_POST_PATCH_HOOKS += MAKE_RESTORE_LIBDIR

# Disable the 'load' operation for static builds since it needs dlopen
ifeq ($(BR2_STATIC_LIBS),y)
MAKE_CONF_OPTS += --disable-load
endif

$(eval $(autotools-package))
$(eval $(host-autotools-package))
