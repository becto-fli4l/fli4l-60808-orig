################################################################################
#
# isdn-firmware
#
################################################################################

ISDN_FIRMWARE_VERSION = 3.2p1
ISDN_FIRMWARE_SITE = ftp://ftp.isdn4linux.de/pub/isdn4linux/utils
ISDN_FIRMWARE_SOURCE = isdn4k-utils.v$(ISDN_FIRMWARE_VERSION).tar.bz2

ISDN_FIRMWARE_FILES += hisax/ISAR.BIN
ISDN_FIRMWARE_LICENSE += GPLv2+
ISDN_FIRMWARE_LICENSE_FILES += COPYING

define ISDN_FIRMWARE_INSTALL_TARGET_CMDS
   $(foreach f,$(ISDN_FIRMWARE_FILES),$(INSTALL) -D -m 644 $(@D)/$(f) $(TARGET_DIR)/lib/firmware/isdn/$(notdir $(f))$(sep))
endef

$(eval $(generic-package))
