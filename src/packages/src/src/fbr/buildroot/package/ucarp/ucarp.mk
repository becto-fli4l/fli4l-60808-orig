UCARP_VERSION = 1.5.2
UCARP_SITE = http://download.pureftpd.org/ucarp
UCARP_SOURCE = ucarp-$(UCARP_VERSION).tar.bz2
UCARP_AUTORECONF = YES

UCARP_DEPENDENCIES = libpcap

UCARP_LICENSE = Proprietary
UCARP_LICENSE_FILES = COPYING

$(eval $(autotools-package))
