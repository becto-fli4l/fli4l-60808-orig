VBOX_VERSION = $(ISDNUTILS_VERSION)
VBOX_SOURCE = $(ISDNUTILS_SOURCE)
VBOX_PATCH = $(ISDNUTILS_PATCH)
VBOX_SITE = $(ISDNUTILS_SITE)
VBOX_SUBDIR = vbox

VBOX_LICENSE = GPLv2+
VBOX_LICENSE_FILES = vbox/COPYING \
                     vbox/README

VBOX_DEPENDENCIES = tcl ncurses
VBOX_CONF_ENV = $(ISDNUTILS_CONF_ENV)
VBOX_CONF_OPTS = --with-tcllib=tcl$(TCL_VERSION_MAJOR)
VBOX_MAKE_OPTS = CFLAGS="$(TARGET_CFLAGS) -DUSE_INTERP_RESULT"

VBOX_PRE_CONFIGURE_HOOKS += ISDNUTILS_PREPARE_CONFIG

define VBOX_TOUCH_MAKEFILE
	touch $(VBOX_DIR)/$(VBOX_SUBDIR)/utils/Makefile.in
	touch $(VBOX_DIR)/$(VBOX_SUBDIR)/doc/Makefile.in
	touch $(VBOX_DIR)/$(VBOX_SUBDIR)/doc/de/Makefile.in
endef
VBOX_PRE_CONFIGURE_HOOKS += VBOX_TOUCH_MAKEFILE

define VBOX_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/$(VBOX_SUBDIR)/src/vboxgetty $(TARGET_DIR)/sbin/vboxgetty
	ln -sf vboxgetty $(TARGET_DIR)/sbin/vboxputty
	$(INSTALL) -D $(@D)/$(VBOX_SUBDIR)/src/vboxd $(TARGET_DIR)/sbin/vboxd
	$(INSTALL) -D $(@D)/$(VBOX_SUBDIR)/src/vboxconvert $(TARGET_DIR)/usr/bin/vboxcnvt
	ln -sf vboxcnvt $(TARGET_DIR)/usr/bin/rmdtovbox
	ln -sf vboxcnvt $(TARGET_DIR)/usr/bin/vboxtoau
	ln -sf vboxcnvt $(TARGET_DIR)/usr/bin/autovbox
	ln -sf vboxcnvt $(TARGET_DIR)/usr/bin/vboxmode
	$(INSTALL) -D $(@D)/$(VBOX_SUBDIR)/src/vboxctrl $(TARGET_DIR)/usr/bin/vboxctrl
	$(INSTALL) -D $(@D)/$(VBOX_SUBDIR)/src/vboxbeep $(TARGET_DIR)/usr/bin/vboxbeep

	$(INSTALL) -D $(@D)/$(VBOX_SUBDIR)/examples/vboxgetty.conf.example $(TARGET_DIR)/etc/isdn/vboxgetty.conf
	$(INSTALL) -D $(@D)/$(VBOX_SUBDIR)/examples/vboxd.conf.example $(TARGET_DIR)/etc/isdn/vboxd.conf
endef

$(eval $(call autotools-package))
