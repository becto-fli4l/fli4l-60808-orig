################################################################################
#
# lpty
#
################################################################################

LPTY_VERSION = 1.2.2-1
LPTY_SUBDIR = lpty-$(LPTY_VERSION)
LPTY_LICENSE = MIT
LPTY_LICENSE_FILES = $(LPTY_SUBDIR)/doc/LICENSE

$(eval $(luarocks-package))
