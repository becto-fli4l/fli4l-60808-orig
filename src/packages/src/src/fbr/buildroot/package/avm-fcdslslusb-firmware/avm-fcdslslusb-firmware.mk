################################################################################
#
# avm-fcdslslusb-firmware
#
################################################################################

AVM_FCDSLSLUSB_FIRMWARE_VERSION = 3.11-07
AVM_FCDSLSLUSB_FIRMWARE_SITE = ftp://ftp.avm.de/cardware/fritzcrd.dsl_sl_usb/linux_64bit/suse.93
AVM_FCDSLSLUSB_FIRMWARE_SOURCE = fcdslslusb-suse93-64bit-$(AVM_FCDSLSLUSB_FIRMWARE_VERSION).tar.gz

AVM_FCDSLSLUSB_FIRMWARE_FILES = fdlubase.frm

AVM_FCDSLSLUSB_FIRMWARE_LICENSE = Proprietary
AVM_FCDSLSLUSB_FIRMWARE_LICENSE_FILES = license.txt

define AVM_FCDSLSLUSB_FIRMWARE_INSTALL_TARGET_CMDS
	$(foreach f,$(AVM_FCDSLSLUSB_FIRMWARE_FILES),$(INSTALL) -D -m 644 $(@D)/$(f) $(TARGET_DIR)/lib/firmware/isdn/$(notdir $(f))$(sep))
endef

$(eval $(generic-package))
