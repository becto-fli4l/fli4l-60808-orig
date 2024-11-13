################################################################################
#
# mtools
#
################################################################################

MTOOLS_VERSION = 4.0.18
MTOOLS_SOURCE = mtools-$(MTOOLS_VERSION).tar.bz2
MTOOLS_SITE = $(BR2_GNU_MIRROR)/mtools
MTOOLS_LICENSE = GPL-3.0+
MTOOLS_LICENSE_FILES = COPYING
MTOOLS_CONF_OPTS = --without-x --disable-floppyd
# info documentation not needed
MTOOLS_CONF_ENV = \
	ac_cv_func_setpgrp_void=yes \
	ac_cv_lib_bsd_gethostbyname=no \
	ac_cv_lib_bsd_main=no \
	ac_cv_path_INSTALL_INFO=

HOST_MTOOLS_CONF_ENV = \
	ac_cv_lib_bsd_gethostbyname=no \
	ac_cv_lib_bsd_main=no \
	ac_cv_path_INSTALL_INFO=

# link with iconv if enabled
ifeq ($(BR2_PACKAGE_LIBICONV),y)
MTOOLS_DEPENDENCIES += libiconv
MTOOLS_CONF_ENV += LIBS=-liconv
endif

# Package does not build in parallel due to improper make rules
MTOOLS_MAKE = $(MAKE1)

define HOST_MTOOLS_DISABLE_ICONV
	sed -i -e 's,^#define HAVE_ICONV_H.*$$,/* #undef HAVE_ICONV_H */,' $(@D)/config.h
endef
HOST_MTOOLS_POST_CONFIGURE_HOOKS += HOST_MTOOLS_DISABLE_ICONV

$(eval $(autotools-package))
$(eval $(host-autotools-package))
