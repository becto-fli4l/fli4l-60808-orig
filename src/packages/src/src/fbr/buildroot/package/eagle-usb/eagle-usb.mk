EAGLE_USB_VERSION = 2.3.3
EAGLE_USB_SOURCE  = eagle-usb-$(EAGLE_USB_VERSION).tar.bz2
EAGLE_USB_SITE    = http://download.gna.org/eagleusb/eagle-usb-2.3.0

EAGLE_USB_LICENSE = GPLv2+
EAGLE_USB_LICENSE_FILES = LICENSE

EAGLE_USB_CONF_ENV = PPPD=yes DHCP=yes PPPOE=yes TCPIP1=false TCPIP2=false PIDOF=/usr/bin/pidof
EAGLE_USB_CONF_OPTS = --with-lang=en --disable-module --disable-eagleconnect --disable-hotplug
EAGLE_USB_MAKE = $(MAKE) CC=$(TARGET_CC) CFLAGS="$(TARGET_CFLAGS)" LDFLAGS="$(TARGET_LDFLAGS)"

$(eval $(autotools-package))
