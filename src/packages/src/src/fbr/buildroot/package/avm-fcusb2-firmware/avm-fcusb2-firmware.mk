################################################################################
#
# avm-fcusb2-firmware
#
################################################################################

AVM_FCUSB2_FIRMWARE_VERSION = 3.11-07
AVM_FCUSB2_FIRMWARE_SITE = ftp://ftp.avm.de/cardware/fritzcrdusb.v20/linux_64bit/suse.10.0
AVM_FCUSB2_FIRMWARE_SOURCE = fcusb2-suse10.0-64bit-$(AVM_FCUSB2_FIRMWARE_VERSION).tar.gz

AVM_FCUSB2_FIRMWARE_FILES = fus2base.frm \
                            fus3base.frm

AVM_FCUSB2_FIRMWARE_LICENSE = Proprietary
AVM_FCUSB2_FIRMWARE_LICENSE_FILES = license.txt

define AVM_FCUSB2_FIRMWARE_INSTALL_TARGET_CMDS
	$(foreach f,$(AVM_FCUSB2_FIRMWARE_FILES),$(INSTALL) -D -m 644 $(@D)/$(f) $(TARGET_DIR)/lib/firmware/isdn/$(notdir $(f))$(sep))
endef

$(eval $(generic-package))
