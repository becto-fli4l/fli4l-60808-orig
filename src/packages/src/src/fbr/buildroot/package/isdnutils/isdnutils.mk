ISDNUTILS_VERSION = 3.25+dfsg1
ISDNUTILS_SOURCE = isdnutils_$(ISDNUTILS_VERSION).orig.tar.bz2
ISDNUTILS_PATCH = isdnutils_$(ISDNUTILS_VERSION)-9.debian.tar.xz
ISDNUTILS_SITE = http://snapshot.debian.org/archive/debian/20171125T033536Z/pool/main/i/isdnutils

ISDNUTILS_LICENSE = GPLv2+, LGPLv2.1+
ISDNUTILS_LICENSE_FILES = README \
                          gpl-2.0.txt \
                          lgpl-2.1.txt

ISDNUTILS_CONF_ENV = I4LVERSION=3.25

define ISDNUTILS_PREPARE_CONFIG
	cp -a package/isdnutils/isdnutils.config $(@D)/.config
	( cd $(@D); perl scripts/mk_autoconf.pl; )
	for c in `find $(@D)/ -name configure`; do \
		head -n 1 $$c > $$c.new; \
		echo ". ../.config" >> $$c.new; \
		sed -e '1d' $$c >> $$c.new; \
		mv $$c.new $$c; \
		chmod +x $$c; \
	done
endef

MAKEFILE_LIST_OLD := $(MAKEFILE_LIST)
include $(sort $(wildcard package/isdnutils/*/*.mk))
MAKEFILE_LIST := $(MAKEFILE_LIST_OLD)
