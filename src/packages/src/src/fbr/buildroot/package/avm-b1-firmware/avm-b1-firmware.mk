################################################################################
#
# avm-b1-firmware
#
################################################################################

AVM_B1_FIRMWARE_SITE = ftp://ftp.in-berlin.de/pub/capi4linux/firmware/b1/3-11-03
AVM_B1_FIRMWARE_SOURCE = b1.t4

AVM_B1_FIRMWARE_LICENSE = Proprietary

define AVM_B1_FIRMWARE_EXTRACT_CMDS
	cp $(DL_DIR)/$(AVM_B1_FIRMWARE_SOURCE) $(@D)
endef

define AVM_B1_FIRMWARE_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 644 $(@D)/$(AVM_B1_FIRMWARE_SOURCE) $(TARGET_DIR)/lib/firmware/isdn/$(notdir $(AVM_B1_FIRMWARE_SOURCE))
endef

$(eval $(generic-package))
