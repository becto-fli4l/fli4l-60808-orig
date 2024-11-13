################################################################################
#
# ath10k-firmware
#
################################################################################

ATH10K_FIRMWARE_VERSION = 7d14e337ad25c0ef3e78fc47eac336697ca612e8
ATH10K_FIRMWARE_SITE = https://github.com/kvalo/ath10k-firmware.git
ATH10K_FIRMWARE_SITE_METHOD = git

# qca988x
ifeq ($(BR2_PACKAGE_ATH10K_FIRMWARE_QCA988X),y)
ATH10K_FIRMWARE_FILES += ath10k/QCA988X/hw2.0/firmware-2.bin
endif

# qca99x0
ifeq ($(BR2_PACKAGE_ATH10K_FIRMWARE_QCA99X0),y)
ATH10K_FIRMWARE_FILES += ath10k/QCA99X0/hw2.0/firmware-5.bin_10.4.1.00007-1
endif

ifneq ($(ATH10K_FIRMWARE_FILES),)

# Most firmware files are under a proprietary license, so no need to
# repeat it for every selections above. Those firmwares that have more
# lax licensing terms may still add them on a per-case basis.
ATH10K_FIRMWARE_LICENSE += Proprietary

# This file contains some licensing information about all the firmware
# files found in the ath10k-firmware package, so we always add it.
ATH10K_FIRMWARE_ALL_LICENSE_FILES += LICENSE.qca_firmware

define ATH10K_FIRMWARE_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/lib/firmware
	$(TAR) c -C $(@D) $(sort $(ATH10K_FIRMWARE_FILES)) | \
		$(TAR) x -C $(TARGET_DIR)/lib/firmware
endef

endif

$(eval $(generic-package))
