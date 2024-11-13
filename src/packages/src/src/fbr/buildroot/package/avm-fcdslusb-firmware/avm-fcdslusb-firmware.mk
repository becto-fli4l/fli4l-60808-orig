################################################################################
#
# avm-fcdslusb-firmware
#
################################################################################

AVM_FCDSLUSB_FIRMWARE_VERSION = 3.11-07
AVM_FCDSLUSB_FIRMWARE_SITE = ftp://ftp.avm.de/cardware/fritzcrd.dsl_usb/linux/suse.93
AVM_FCDSLUSB_FIRMWARE_SOURCE = fcdslusb-suse93-$(AVM_FCDSLUSB_FIRMWARE_VERSION).tar.gz

AVM_FCDSLUSB_FIRMWARE_FILES = 1/fritz/fdsubase.frm \
                              2/fritz/fds2base.frm

AVM_FCDSLUSB_FIRMWARE_LICENSE = Proprietary
AVM_FCDSLUSB_FIRMWARE_LICENSE_FILES = license.txt

define AVM_FCDSLUSB_FIRMWARE_INSTALL_TARGET_CMDS
	$(foreach f,$(AVM_FCDSLUSB_FIRMWARE_FILES),$(INSTALL) -D -m 644 $(@D)/$(f) $(TARGET_DIR)/lib/firmware/isdn/$(notdir $(f))$(sep))
endef

$(eval $(generic-package))
