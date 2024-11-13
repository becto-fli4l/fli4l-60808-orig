################################################################################
#
# b43legacy-firmware
#
################################################################################

B43LEGACY_FIRMWARE_VERSION = 3.130.20.0
B43LEGACY_FIRMWARE_SITE = http://downloads.openwrt.org/sources
B43LEGACY_FIRMWARE_SOURCE = wl_apsta-$(B43LEGACY_FIRMWARE_VERSION).o

B43LEGACY_FIRMWARE_LICENSE = PROPRIETARY
B43LEGACY_FIRMWARE_REDISTRIBUTE = NO

B43LEGACY_FIRMWARE_DEPENDENCIES = host-b43-fwcutter

define B43LEGACY_FIRMWARE_EXTRACT_CMDS
   cp $(DL_DIR)/$(B43LEGACY_FIRMWARE_SOURCE) $(@D)
endef

define B43LEGACY_FIRMWARE_INSTALL_TARGET_CMDS
	$(INSTALL) -d -m 0755 $(TARGET_DIR)/lib/firmware
	$(HOST_DIR)/bin/b43-fwcutter -w $(TARGET_DIR)/lib/firmware/ $(@D)/$(B43LEGACY_FIRMWARE_SOURCE)
endef

$(eval $(generic-package))
