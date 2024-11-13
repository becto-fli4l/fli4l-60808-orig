################################################################################
#
# i4l-firmware
#
################################################################################

I4L_FIRMWARE_VERSION = 2012.9.28-36.3
I4L_FIRMWARE_SITE = ftp://ftp.pbone.net/mirror/ftp5.gwdg.de/pub/opensuse/repositories/home:/enzokiel/openSUSE_12.2_Update/x86_64
I4L_FIRMWARE_SOURCE = i4lfirm-$(I4L_FIRMWARE_VERSION).x86_64.rpm

I4L_FIRMWARE_LICENSE = GPLv2+

# ICN 2B
ifeq ($(BR2_PACKAGE_I4L_FIRMWARE_ICN2B),y)
I4L_FIRMWARE_FILES += lib/firmware/isdn/loadpg.bin \
							 lib/firmware/isdn/pc_eu_ca.bin
endif

define I4L_FIRMWARE_INSTALL_TARGET_CMDS
   mkdir -p $(TARGET_DIR)/lib/firmware/isdn
   install -m 664 $(addprefix $(@D)/,$(I4L_FIRMWARE_FILES)) $(TARGET_DIR)/lib/firmware/isdn
endef

$(eval $(generic-package))
