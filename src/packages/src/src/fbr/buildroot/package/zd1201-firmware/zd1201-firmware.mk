################################################################################
#
# zd1201-firmware
#
################################################################################

ZD1201_FIRMWARE_VERSION = 0.14
ZD1201_FIRMWARE_SITE = http://downloads.sourceforge.net/project/linux-lc100020/%28NEW%29%20zd1201%20driver/zd1201.%20Version%200.14
ZD1201_FIRMWARE_SOURCE = zd1201-$(ZD1201_FIRMWARE_VERSION)-fw.tar.gz
ZD1201_FIRMWARE_FILES += zd1201-ap.fw zd1201.fw

define ZD1201_FIRMWARE_INSTALL_TARGET_CMDS
   mkdir -p $(TARGET_DIR)/lib/firmware
   $(TAR) c -C $(@D) $(ZD1201_FIRMWARE_FILES) | \
      $(TAR) x -C $(TARGET_DIR)/lib/firmware

endef

$(eval $(generic-package))
