LPRNG_VERSION = 3.8.35
LPRNG_SOURCE  = LPRng-$(LPRNG_VERSION).tgz
LPRNG_SITE    = ftp://ftp.lprng.com/pub/LPRng/LPRng

LPRNG_LICENSE = BSD-4c, BSD-3c, GPLv2+
LPRNG_LICENSE_FILES = COPYRIGHT \
                      LICENSE

LPRNG_MAKE = $(MAKE1)
LPRNG_CONF_OPTS = --disable-ssl --disable-kerberos --disable-setuid \
	--disable-werror --disable-require_configfiles --disable-rpath \
	--disable-strip --with-userid=lp --with-groupid=lp
LPRNG_CONF_ENV = CLEAR=no

define LPRNG_CLEAN_SOURCES
	find $(@D) -name '*.rej' -exec rm \{\} +
endef

LPRNG_POST_EXTRACT_HOOKS += LPRNG_CLEAN_SOURCES

$(eval $(autotools-package))
