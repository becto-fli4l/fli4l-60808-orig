METALOG_VERSION = 3
METALOG_SOURCE  = metalog-$(METALOG_VERSION).tar.xz
METALOG_SITE    = http://downloads.sourceforge.net/project/metalog

METALOG_LICENSE = GPLv2+
METALOG_LICENSE_FILES = COPYING

METALOG_DEPENDENCIES = pcre

$(eval $(autotools-package))
