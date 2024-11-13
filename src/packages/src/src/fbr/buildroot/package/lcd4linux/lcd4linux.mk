LCD4LINUX_VERSION     = 1191
LCD4LINUX_DL_VERSION  = $(LCD4LINUX_VERSION)
LCD4LINUX_SITE_METHOD = svn
LCD4LINUX_SITE        = https://ssl.bulix.org/svn/lcd4linux/trunk
LCD4LINUX_SOURCE      = lcd4linux-$(LCD4LINUX_VERSION).tar.gz

LCD4LINUX_LICENSE = GPLv2+
LCD4LINUX_LICENSE_FILES = COPYING

LCD4LINUX_AUTORECONF  = YES
LCD4LINUX_DEPENDENCIES = serdisplib gd jpeg libusb dpflib ncurses

LCD4LINUX_CONF_OPTS = --without-x --without-python --with-ncurses=$(STAGING_DIR)/usr \
	--with-drivers=all,\!X11,\!HD44780-I2C,\!LCDLinux,\!LUIse,\!MDM166A,\!st2205,\!ULA200,\!VNC \
	--with-plugins=all,\!xmms,\!dbus,\!gps,\!mpd,\!mpris_dbus,\!mysql,\!qnaplog \
	--disable-rpath

define LCD4LINUX_CREATE_M4
	mkdir -p $(@D)/m4
endef
define LCD4LINUX_CORRECT_VERSION
	sed -i -e "s/SVN_VERSION \"[^\"]*\"/SVN_VERSION \"$(LCD4LINUX_VERSION)\"/" $(@D)/svn_version.h
endef
LCD4LINUX_POST_EXTRACT_HOOKS += LCD4LINUX_CREATE_M4 LCD4LINUX_CORRECT_VERSION

$(eval $(autotools-package))
