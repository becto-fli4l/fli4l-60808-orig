#############################################################
#
# knot - a dns server
#
#############################################################

KNOT_VERSION = 1.6.8
KNOT_SOURCE  = knot-$(KNOT_VERSION).tar.gz
KNOT_SITE    = https://secure.nic.cz/files/knot-dns

KNOT_LICENSE = GPLv3+
KNOT_LICENSE_FILES = COPYING

# the package dependencies
KNOT_DEPENDENCIES = openssl liburcu host-bison

ifeq ($(BR2_ARCH_IS_64),y)
KNOT_CONF_OPTS += --enable-force64bits
else
KNOT_CONF_OPTS += --enable-force32bits
endif

$(eval $(autotools-package))
