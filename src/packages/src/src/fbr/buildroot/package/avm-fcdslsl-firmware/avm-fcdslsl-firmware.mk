################################################################################
#
# avm-fcdslsl-firmware
#
################################################################################

AVM_FCDSLSL_FIRMWARE_VERSION = 3.11-07
AVM_FCDSLSL_FIRMWARE_SITE = ftp://ftp.avm.de/cardware/fritzcrd.dsl_sl/linux_64bit/suse.93
AVM_FCDSLSL_FIRMWARE_SOURCE = fcdslsl-suse93-64bit-$(AVM_FCDSLSL_FIRMWARE_VERSION).tar.gz

AVM_FCDSLSL_FIRMWARE_FILES = fdssbase.bin

AVM_FCDSLSL_FIRMWARE_LICENSE = Proprietary
AVM_FCDSLSL_FIRMWARE_LICENSE_FILES = license.txt

define AVM_FCDSLSL_FIRMWARE_INSTALL_TARGET_CMDS
	$(foreach f,$(AVM_FCDSLSL_FIRMWARE_FILES),$(INSTALL) -D -m 644 $(@D)/$(f) $(TARGET_DIR)/lib/firmware/isdn/$(notdir $(f))$(sep))
endef

$(eval $(generic-package))
