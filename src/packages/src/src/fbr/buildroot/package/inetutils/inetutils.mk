INETUTILS_VERSION = 1.9.4
INETUTILS_SOURCE  = inetutils-$(INETUTILS_VERSION).tar.xz
INETUTILS_SITE    = ftp://ftp.gnu.org/gnu/inetutils

INETUTILS_LICENSE = GPLv3+
INETUTILS_LICENSE_FILES = COPYING

INETUTILS_DEPENDENCIES = ncurses readline

INETUTILS_MAKE_ENV = CC=$(TARGET_CC) LD=$(TARGET_LD)
INETUTILS_CONF_OPTS = \
	--disable-servers \
	--disable-clients \
	--disable-libls \
	--disable-rpath \
	--enable-ftp \
	--with-libreadline-prefix=$(STAGING_DIR)/usr

define INETUTILS_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/ftp/ftp $(TARGET_DIR)/usr/bin/ftp
endef

$(eval $(autotools-package))
