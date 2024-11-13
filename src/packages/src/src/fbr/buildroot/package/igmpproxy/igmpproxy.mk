################################################################################
#
# igmpproxy
#
################################################################################

IGMPPROXY_VERSION = 0.2.1
IGMPPROXY_SITE = $(call github,pali,igmpproxy,$(IGMPPROXY_VERSION))
IGMPPROXY_AUTORECONF = YES
IGMPPROXY_LICENSE = GPL-2.0+, BSD-3-Clause (mrouted)
IGMPPROXY_LICENSE_FILES = COPYING GPL.txt Stanford.txt
# http://wl500g.googlecode.com/svn/!svn/bc/5066/trunk/igmpproxy/
# Rev. 5066 corresponds to 1.9.2.7-rtn-r5066,
# see https://code.google.com/p/wl500g/wiki/News

$(eval $(autotools-package))
