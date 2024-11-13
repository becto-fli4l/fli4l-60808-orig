DIBBLER_VERSION = 1.0.1
DIBBLER_SITE    = http://klub.com.pl/dhcpv6/dibbler
DIBBLER_SOURCE  = dibbler-$(DIBBLER_VERSION).tar.gz

DIBBLER_LICENSE = GPLv2+
DIBBLER_LICENSE_FILES = LICENSE

# allow multiple clients running simultaneously on a single host
DIBBLER_CONF_OPTS = CFLAGS="$(TARGET_CFLAGS) -DMOD_CLNT_BIND_REUSE"

$(eval $(autotools-package))
