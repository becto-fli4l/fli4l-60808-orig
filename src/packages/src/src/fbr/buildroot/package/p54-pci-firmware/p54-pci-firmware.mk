################################################################################
#
# p54-pci-firmware
#
################################################################################

P54_PCI_FIRMWARE_VERSION = 2.17.2.0
P54_PCI_FIRMWARE_SITE = https://daemonizer.de/prism54/prism54-fw/fw-softmac
P54_PCI_FIRMWARE_SOURCE = $(P54_PCI_FIRMWARE_VERSION).arm

define P54_PCI_FIRMWARE_EXTRACT_CMDS
   cp $(DL_DIR)/$(P54_PCI_FIRMWARE_SOURCE) $(@D)/isl3886pci
endef

define P54_PCI_FIRMWARE_INSTALL_TARGET_CMDS
   mkdir -p $(TARGET_DIR)/lib/firmware
   install -m 664 $(@D)/isl3886pci $(TARGET_DIR)/lib/firmware

endef

$(eval $(generic-package))
