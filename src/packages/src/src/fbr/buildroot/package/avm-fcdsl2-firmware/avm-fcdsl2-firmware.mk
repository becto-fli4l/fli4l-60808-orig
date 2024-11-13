################################################################################
#
# avm-fcdsl2-firmware
#
################################################################################

AVM_FCDSL2_FIRMWARE_VERSION = 3.11-07
AVM_FCDSL2_FIRMWARE_SITE = ftp://ftp.avm.de/cardware/fritzcrd.dsl_v20/linux_64bit/suse.10.0
AVM_FCDSL2_FIRMWARE_SOURCE = fcdsl2-suse10.0-64bit-$(AVM_FCDSL2_FIRMWARE_VERSION).tar.gz

AVM_FCDSL2_FIRMWARE_FILES = fds2base.bin

AVM_FCDSL2_FIRMWARE_LICENSE = Proprietary
AVM_FCDSL2_FIRMWARE_LICENSE_FILES = license.txt

define AVM_FCDSL2_FIRMWARE_INSTALL_TARGET_CMDS
	$(foreach f,$(AVM_FCDSL2_FIRMWARE_FILES),$(INSTALL) -D -m 644 $(@D)/$(f) $(TARGET_DIR)/lib/firmware/isdn/$(notdir $(f))$(sep))
endef

$(eval $(generic-package))
