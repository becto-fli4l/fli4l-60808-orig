################################################################################
#
# netio
#
################################################################################

NETIO_VERSION = 1.32
NETIO_SITE = http://www.ars.de/ARS/ars.nsf/f24a6a0b94c22d82862566960071bf5a/aa577bc4be573b05c125706d004c75b5/$$FILE
NETIO_SOURCE = netio$(subst .,,$(NETIO_VERSION)).zip
NETIO_LICENSE = Proprietary
NETIO_DEPENDENCIES = host-dos2unix

define NETIO_BUILD_CMDS
	# see Gentoo ebuild
	$(HOST_DIR)/bin/dos2unix $(@D)/*.c $(@D)/*.h $(@D)/Makefile
	$(SED) "s|LFLAGS=\"\"|LFLAGS?=\"${LDFLAGS}\"|g" \
		-e 's|\(CC\)=|\1?=|g' \
		-e 's|\(CFLAGS\)=|\1+=|g' \
		$(@D)/Makefile
	+$(MAKE) -C $(@D) CC=$(TARGET_CC) CFLAGS="$(TARGET_CFLAGS)" linux
endef

define NETIO_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/netio $(TARGET_DIR)/usr/bin/netio
endef

$(eval $(generic-package))
