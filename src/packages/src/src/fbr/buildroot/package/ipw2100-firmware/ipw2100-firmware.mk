################################################################################
#
# ipw2100-firmware
#
################################################################################

IPW2100_FIRMWARE_VERSION = 1.3
IPW2100_FIRMWARE_SITE = http://pkgs.fedoraproject.org/repo/pkgs/ipw2100-firmware/ipw2100-fw-$(IPW2100_FIRMWARE_VERSION).tgz/46aa75bcda1a00efa841f9707bbbd113
IPW2100_FIRMWARE_SOURCE = ipw2100-fw-$(IPW2100_FIRMWARE_VERSION).tgz

IPW2100_FIRMWARE_FILES += ipw2100-1.3.fw   \
                          ipw2100-1.3-i.fw \
                          ipw2100-1.3-p.fw

IPW2100_FIRMWARE_LICENSE = Proprietary
IPW2100_FIRMWARE_LICENSE_FILES += LICENSE

define IPW2100_FIRMWARE_EXTRACT_CMDS
$(INFLATE$(suffix $(IPW2100_FIRMWARE_SOURCE))) $(DL_DIR)/$(IPW2100_FIRMWARE_SOURCE) | \
   $(TAR) -C $(@D) $(TAR_OPTIONS) -
endef

define IPW2100_FIRMWARE_INSTALL_TARGET_CMDS
   mkdir -p $(TARGET_DIR)/lib/firmware
   $(TAR) c -C $(@D) $(IPW2100_FIRMWARE_FILES) | \
      $(TAR) x -C $(TARGET_DIR)/lib/firmware
endef

$(eval $(generic-package))
