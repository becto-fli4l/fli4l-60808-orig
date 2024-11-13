APCUPSD_VERSION = 3.14.14
APCUPSD_SOURCE = apcupsd-$(APCUPSD_VERSION).tar.gz
APCUPSD_SITE = http://downloads.sourceforge.net/project/apcupsd/apcupsd%20-%20Stable/$(APCUPSD_VERSION)

APCUPSD_LICENSE = GPLv2
APCUPSD_LICENSE_FILES = COPYING

APCUPSD_CONF_OPTS = --enable-cgi --enable-usb --disable-gapcmon --disable-snmp \
                    --disable-net-snmp --with-cgi-bin=/srv/www/cgi-bin \
                    --with-nologin=/etc/apcupsd --with-log-dir=/var/log \
                    --with-lock-dir=/var/lock --with-powerfail-dir=/etc/apcupsd --without-x \
                    --with-distname=fli4l --sysconfdir=/etc/apcupsd

APCUPSD_DEPENDENCIES = gd

APCUPSD_CONF_ENV =  ac_cv_func_setpgrp_void=yes LD=$(TARGET_CC) \
					ac_cv_path_WALL=/usr/bin/wall \
					ac_cv_path_SHUTDOWN=/sbin/shutdown

define APCUPSD_ADD_FLI4L_DISTRI
	cp -dR $(@D)/platforms/unknown $(@D)/platforms/fli4l
endef
APCUPSD_POST_EXTRACT_HOOKS += APCUPSD_ADD_FLI4L_DISTRI

$(eval $(autotools-package))
