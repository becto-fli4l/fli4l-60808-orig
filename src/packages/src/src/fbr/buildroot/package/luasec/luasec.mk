################################################################################
#
# luasec
#
################################################################################

LUASEC_VERSION = 0.6-2
LUASEC_SUBDIR = luasec
LUASEC_LICENSE = MIT
LUASEC_LICENSE_FILES = $(LUASEC_SUBDIR)/LICENSE
LUASEC_DEPENDENCIES = openssl luasocket

$(eval $(luarocks-package))
