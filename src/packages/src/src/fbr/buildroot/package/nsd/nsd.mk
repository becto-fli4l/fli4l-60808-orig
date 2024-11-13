#############################################################
#
# nsd - a dns server
#
#############################################################

NSD_VERSION = 4.1.1
NSD_SOURCE  = nsd-$(NSD_VERSION).tar.gz
NSD_SITE    = http://www.nlnetlabs.nl/downloads/nsd
# NSD_INSTALL_STAGING = YES

NSD_LICENSE = BSD-3c
NSD_LICENSE_FILES = LICENSE

# the package dependencies
NSD_DEPENDENCIES = openssl libevent
NSD_CONF_OPTS = --with-user=dns --with-libevent=$(STAGING_DIR)/usr --with-ssl=$(STAGING_DIR)/usr \
		--disable-flto

ifeq ($(BR2_ARCH_IS_64),y)
NSD_CONF_OPTS += --enable-force64bits
else
NSD_CONF_OPTS += --enable-force32bits
endif

$(eval $(autotools-package))
