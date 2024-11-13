################################################################################
#
# p54-usb-firmware
#
################################################################################

P54_USB_FIRMWARE_VERSION = 2.13.25.0
P54_USB_FIRMWARE_SITE = https://daemonizer.de/prism54/prism54-fw/fw-usb
P54_USB_FIRMWARE_SOURCE = $(P54_USB_FIRMWARE_VERSION).lm86.arm
P54_USB_FIRMWARE_EXTRA_DOWNLOADS = $(P54_USB_FIRMWARE_VERSION).lm87.arm

define P54_USB_FIRMWARE_EXTRACT_CMDS
   cp $(DL_DIR)/$(P54_USB_FIRMWARE_SOURCE) $(@D)/isl3886usb
   cp $(DL_DIR)/$(P54_USB_FIRMWARE_EXTRA_DOWNLOADS) $(@D)/isl3887usb
endef

define P54_USB_FIRMWARE_INSTALL_TARGET_CMDS
   mkdir -p $(TARGET_DIR)/lib/firmware
   install -m 664 $(@D)/isl3886usb $(TARGET_DIR)/lib/firmware
   install -m 664 $(@D)/isl3887usb $(TARGET_DIR)/lib/firmware

endef

$(eval $(generic-package))
