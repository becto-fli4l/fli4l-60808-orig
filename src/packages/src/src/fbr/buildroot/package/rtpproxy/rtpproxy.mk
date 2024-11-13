#############################################################
#
# rtpproxy
#
#############################################################

RTPPROXY_VERSION = 1.2.1
RTPPROXY_SOURCE  = rtpproxy-$(RTPPROXY_VERSION).tar.gz
RTPPROXY_SITE    = http://b2bua.org/chrome/site

RTPPROXY_LICENSE = BSD-2c
RTPPROXY_LICENSE_FILES = COPYING

# the package dependencies
RTPPROXY_DEPENDENCIES = host-pkgconf

$(eval $(autotools-package))
