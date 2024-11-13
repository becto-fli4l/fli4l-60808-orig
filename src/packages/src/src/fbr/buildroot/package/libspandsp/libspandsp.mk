################################################################################
#
# libspandsp
#
################################################################################

LIBSPANDSP_VERSION = 20180108
LIBSPANDSP_SOURCE = spandsp-$(LIBSPANDSP_VERSION).tar.gz
LIBSPANDSP_SITE = http://www.soft-switch.org/downloads/spandsp/snapshots
LIBSPANDSP_LICENSE = LGPLv2.1
LIBSPANDSP_LICENSE_FILES = COPYING
LIBSPANDSP_INSTALL_STAGING = YES
LIBSPANDSP_AUTORECONF = YES

# Fixes broken gcc
# http://thread.gmane.org/gmane.linux.lib.musl.general/2468/
LIBSPANDSP_CONF_ENV = ac_cv_header_tgmath_h=no

LIBSPANDSP_DEPENDENCIES = tiff

ifeq ($(BR2_PACKAGE_JPEG),y)
LIBSPANDSP_DEPENDENCIES += jpeg
endif

$(eval $(autotools-package))
