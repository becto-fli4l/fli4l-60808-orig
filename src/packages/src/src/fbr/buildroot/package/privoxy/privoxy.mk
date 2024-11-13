################################################################################
#
# privoxy
#
################################################################################

PRIVOXY_VERSION = 3.0.32
PRIVOXY_SITE = http://downloads.sourceforge.net/project/ijbswa/Sources/$(PRIVOXY_VERSION)%20%28stable%29
PRIVOXY_SOURCE = privoxy-$(PRIVOXY_VERSION)-stable-src.tar.gz
PRIVOXY_CONF_OPTS = --enable-no-gifs
# configure not shipped
PRIVOXY_AUTORECONF = YES
PRIVOXY_DEPENDENCIES = pcre zlib
PRIVOXY_LICENSE = GPL-2.0+
PRIVOXY_LICENSE_FILES = LICENSE

define PRIVOXY_REMOVE_LOGFILE
	rm $(TARGET_DIR)/var/log/privoxy/logfile
endef
PRIVOXY_POST_INSTALL_TARGET_HOOKS += PRIVOXY_REMOVE_LOGFILE

$(eval $(autotools-package))
