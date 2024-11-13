################################################################################
#
# rsync
#
################################################################################

RSYNC_VERSION = 3.1.3
RSYNC_SITE = http://rsync.samba.org/ftp/rsync/src
RSYNC_LICENSE = GPL-3.0+
RSYNC_LICENSE_FILES = COPYING
RSYNC_DEPENDENCIES = zlib popt libiconv
RSYNC_CONF_OPTS = \
	--with-included-zlib=no \
	--with-included-popt=no

ifeq ($(BR2_PACKAGE_ACL),y)
RSYNC_DEPENDENCIES += acl
else
RSYNC_CONF_OPTS += --disable-acl-support
endif

define RSYNC_FIX_NOGROUP
	sed -ie 's/\(^#define NOBODY_USER \).*/\1 "nobody"/' $(@D)/config.h
	sed -ie 's/\(^#define NOBODY_GROUP \).*/\1 "nogroup"/' $(@D)/config.h
endef

RSYNC_POST_CONFIGURE_HOOKS += RSYNC_FIX_NOGROUP

$(eval $(autotools-package))
