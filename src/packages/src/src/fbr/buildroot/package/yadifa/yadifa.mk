#############################################################
#
# yadifa - a dns server
#
#############################################################

YADIFA_VERSION = 2.2.5-6937
YADIFA_SOURCE  = yadifa-$(YADIFA_VERSION).tar.gz
YADIFA_SITE    = http://cdn.yadifa.eu/sites/default/files/releases

# the package dependencies
YADIFA_DEPENDENCIES = openssl libpcap
YADIFA_CONF_OPTS = --disable-rpath \
	--enable-master \
	--disable-rrsig-management \
	--disable-dynupdate \
	--enable-non-aa-axfr-support \
	--enable-ctrl \
	--prefix= \
	--enable-messages

YADIFA_LICENSE = BSD-3c
YADIFA_LICENSE_FILES = COPYING
YADIFA_AUTORECONF = YES

$(eval $(autotools-package))
