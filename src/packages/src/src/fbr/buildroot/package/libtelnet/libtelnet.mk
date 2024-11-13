################################################################################
#
# libtelnet
#
################################################################################

LIBTELNET_VERSION = 0.21
LIBTELNET_SITE = https://github.com/seanmiddleditch/libtelnet/archive
LIBTELNET_SOURCE = $(LIBTELNET_VERSION).tar.gz
LIBTELNET_INSTALL_STAGING = YES

# We're patching Makefile.am
LIBTELNET_AUTORECONF = YES
LIBTELNET_LICENSE = Public domain
LIBTELNET_LICENSE_FILES = COPYING

LIBTELNET_CONF_OPTS = --disable-util
LIBTELNET_DEPENDENCIES = zlib

$(eval $(autotools-package))
