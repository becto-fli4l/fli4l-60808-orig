################################################################################
#
# avm-fcdsl-firmware
#
################################################################################

AVM_FCDSL_FIRMWARE_VERSION = 3.11-07
AVM_FCDSL_FIRMWARE_SITE = ftp://ftp.avm.de/cardware/fritzcrd.dsl/linux/suse.93
AVM_FCDSL_FIRMWARE_SOURCE = fcdsl-suse93-$(AVM_FCDSL_FIRMWARE_VERSION).tar.gz

AVM_FCDSL_FIRMWARE_FILES = fdslbase.bin

AVM_FCDSL_FIRMWARE_LICENSE = Proprietary
AVM_FCDSL_FIRMWARE_LICENSE_FILES = license.txt

define AVM_FCDSL_FIRMWARE_INSTALL_TARGET_CMDS
	$(foreach f,$(AVM_FCDSL_FIRMWARE_FILES),$(INSTALL) -D -m 644 $(@D)/$(f) $(TARGET_DIR)/lib/firmware/isdn/$(notdir $(f))$(sep))
endef

$(eval $(generic-package))
