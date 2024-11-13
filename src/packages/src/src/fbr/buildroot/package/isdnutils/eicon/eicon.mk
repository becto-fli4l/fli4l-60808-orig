EICON_VERSION = $(ISDNUTILS_VERSION)
EICON_SOURCE = $(ISDNUTILS_SOURCE)
EICON_PATCH = $(ISDNUTILS_PATCH)
EICON_SITE = $(ISDNUTILS_SITE)
EICON_SUBDIR = eicon

EICON_LICENSE = GPLv2+
EICON_LICENSE_FILES = eicon/README

EICON_DEPENDENCIES = ncurses
EICON_CONF_ENV = $(ISDNUTILS_CONF_ENV)
EICON_MAKE_OPTS = CFLAGS="$(TARGET_CFLAGS)"
EICON_MAKE = $(MAKE1)

EICON_PRE_CONFIGURE_HOOKS += ISDNUTILS_PREPARE_CONFIG

define EICON_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/$(EICON_SUBDIR)/eiconctrl $(TARGET_DIR)/sbin/eiconctrl
	$(INSTALL) -D $(@D)/$(EICON_SUBDIR)/divaload $(TARGET_DIR)/sbin/divaload
	$(INSTALL) -D $(@D)/$(EICON_SUBDIR)/divalogd $(TARGET_DIR)/sbin/divalogd
	$(INSTALL) -D $(@D)/$(EICON_SUBDIR)/divalog $(TARGET_DIR)/sbin/divalog
endef

$(eval $(call autotools-package))
