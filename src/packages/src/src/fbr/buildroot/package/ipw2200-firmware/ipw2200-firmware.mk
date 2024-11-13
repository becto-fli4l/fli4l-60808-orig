################################################################################
#
# ipw2200-firmware
#
################################################################################

IPW2200_FIRMWARE_VERSION = 3.1
IPW2200_FIRMWARE_SITE = http://pkgs.fedoraproject.org/repo/pkgs/ipw2200-firmware/ipw2200-fw-$(IPW2200_FIRMWARE_VERSION).tgz/eaba788643c7cc7483dd67ace70f6e99
IPW2200_FIRMWARE_SOURCE = ipw2200-fw-$(IPW2200_FIRMWARE_VERSION).tgz

IPW2200_FIRMWARE_FILES += ipw2200-bss.fw  \
                          ipw2200-ibss.fw \
                          ipw2200-sniffer.fw

IPW2200_FIRMWARE_LICENSE = Proprietary
IPW2200_FIRMWARE_LICENSE_FILES += LICENSE.ipw2200-fw

define IPW2200_FIRMWARE_INSTALL_TARGET_CMDS
   mkdir -p $(TARGET_DIR)/lib/firmware
   $(TAR) c -C $(@D) $(IPW2200_FIRMWARE_FILES) | \
      $(TAR) x -C $(TARGET_DIR)/lib/firmware
endef

$(eval $(generic-package))
