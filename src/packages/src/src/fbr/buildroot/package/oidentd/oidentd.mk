OIDENTD_VERSION = 2.0.8
OIDENTD_SOURCE  = oidentd-$(OIDENTD_VERSION).tar.gz
OIDENTD_SITE    = http://downloads.sourceforge.net/project/ojnk/oidentd/$(OIDENTD_VERSION)

OIDENTD_LICENSE = GPLv2+, LGPLv2.1+
OIDENTD_LICENSE_FILES = COPYING

OIDENTD_DEPENDENCIES = host-flex host-bison

OIDENTD_CONF_OPTS = --enable-nat --enable-ipv6

$(eval $(autotools-package))
