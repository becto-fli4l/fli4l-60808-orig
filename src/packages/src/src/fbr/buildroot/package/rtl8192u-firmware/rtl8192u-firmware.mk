################################################################################
#
# rtl8192u-firmware
#
################################################################################

RTL8192U_FIRMWARE_VERSION = 1.0-1.1
RTL8192U_FIRMWARE_SITE = http://download.opensuse.org/repositories/driver:/wireless/openSUSE_11.4/noarch
RTL8192U_FIRMWARE_SOURCE = rtl8192u-firmware-$(RTL8192U_FIRMWARE_VERSION).noarch.rpm

RTL8192U_FIRMWARE_LICENSE = Proprietary
RTL8192U_FIRMWARE_LICENSE_FILES = lib/firmware/RTL8192U/LICENCE

define RTL8192U_FIRMWARE_INSTALL_TARGET_CMDS
   mkdir -p $(TARGET_DIR)/lib/firmware/RTL8192U
   install -m 664 $(@D)/lib/firmware/RTL8192U/boot.img $(TARGET_DIR)/lib/firmware/RTL8192U
   install -m 664 $(@D)/lib/firmware/RTL8192U/data.img $(TARGET_DIR)/lib/firmware/RTL8192U
   install -m 664 $(@D)/lib/firmware/RTL8192U/main.img $(TARGET_DIR)/lib/firmware/RTL8192U
endef

$(eval $(generic-package))
