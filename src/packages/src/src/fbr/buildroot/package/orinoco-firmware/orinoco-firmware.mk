################################################################################
#
# orinoco-firmware
#
################################################################################

# taken from orinoco-fwutils 0.3 found at
# http://downloads.sourceforge.net/project/orinoco/orinoco-fwutils/0.3/orinoco-fwutils-0.3.tar.gz?r=http%3A%2F%2Fsourceforge.net%2Fprojects%2Forinoco%2Ffiles%2Forinoco-fwutils%2F0.3%2F&ts=1412800447&use_mirror=cznic

ORINOCO_FIRMWARE_VERSION = SR0201
ORINOCO_FIRMWARE_SITE = ftp://ftp.avaya.com/incoming/Up1cku9/tsoweb/avayawireless
ORINOCO_FIRMWARE_SOURCE = AV_WINXP_PC_USB_$(ORINOCO_FIRMWARE_VERSION).zip

define ORINOCO_FIRMWARE_EXTRACT_CMDS
   unzip -p $(DL_DIR)/$($(PKG)_SOURCE) USB/WLAGS51B.sys >$(@D)/ezusb.drv
	dd if=$(@D)/ezusb.drv of=$(@D)/orinoco_ezusb_fw skip=9983 count=436 bs=16
endef

define ORINOCO_FIRMWARE_INSTALL_TARGET_CMDS
   mkdir -p $(TARGET_DIR)/lib/firmware
   install -m 664 $(@D)/orinoco_ezusb_fw $(TARGET_DIR)/lib/firmware/
endef

$(eval $(generic-package))
