################################################################################
#
# linux-firmware
#
################################################################################

LINUX_FIRMWARE_VERSION = fa0efeff4894e36b9c3964376f2c99fae101d147
LINUX_FIRMWARE_SITE = http://git.kernel.org/pub/scm/linux/kernel/git/firmware/linux-firmware.git
LINUX_FIRMWARE_SITE_METHOD = git

# Intel SST DSP
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_INTEL_SST_DSP),y)
LINUX_FIRMWARE_FILES += intel/fw_sst_0f28.bin-48kHz_i2s_master
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.fw_sst_0f28
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_I915),y)
LINUX_FIRMWARE_DIRS += i915
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENSE.i915
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_RADEON),y)
LINUX_FIRMWARE_DIRS += radeon
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENSE.radeon
endif

# Intel Wireless Bluetooth
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_IBT),y)
LINUX_FIRMWARE_FILES += intel/ibt-*
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.ibt_firmware
endif

# Qualcomm Atheros Rome 6174A Bluetooth
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_QUALCOMM_6174A_BT),y)
LINUX_FIRMWARE_FILES += qca/rampatch_usb_00000302.bin qca/nvm_usb_00000302.bin
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENSE.qcom
endif

# Freescale i.MX SDMA
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_IMX_SDMA),y)
LINUX_FIRMWARE_FILES += imx/sdma/sdma-imx6q.bin imx/sdma/sdma-imx7d.bin
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENSE.sdma_firmware
endif

# rt2501/rt61
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_RALINK_RT61),y)
LINUX_FIRMWARE_FILES += rt2561.bin rt2561s.bin rt2661.bin
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.ralink-firmware.txt
endif

# rt73
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_RALINK_RT73),y)
LINUX_FIRMWARE_FILES += rt73.bin
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.ralink-firmware.txt
endif

# rt2xx
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_RALINK_RT2XX),y)
LINUX_FIRMWARE_FILES += rt2860.bin rt2870.bin rt3071.bin
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.ralink-firmware.txt
define LINUX_FIRMWARE_LINK_RALINK_RT2XX
	ln -sf rt2860.bin $(TARGET_DIR)/lib/firmware/rt3090.bin
	ln -sf rt2870.bin $(TARGET_DIR)/lib/firmware/rt3070.bin
endef
LINUX_FIRMWARE_POST_INSTALL_TARGET_HOOKS += LINUX_FIRMWARE_LINK_RALINK_RT2XX
endif

# rsi_usb
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_RSI_91X),y)
LINUX_FIRMWARE_FILES += rsi_91x.fw
# No license file; the license is in the file WHENCE
# which is installed unconditionally
endif

# rtl8152/8153 usb adapters
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_RTL_8152),y)
LINUX_FIRMWARE_FILES += rtl_nic/rtl8153a-2.fw \
                        rtl_nic/rtl8153a-3.fw \
                        rtl_nic/rtl8153a-4.fw \
                        rtl_nic/rtl8153b-2.fw
# No license file; the license is in the file WHENCE
# which is installed unconditionally
endif

# rtl81xx
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_RTL_81XX),y)
LINUX_FIRMWARE_FILES += \
	rtlwifi/rtl8192cfw.bin rtlwifi/rtl8192cfwU.bin \
	rtlwifi/rtl8192cfwU_B.bin rtlwifi/rtl8192cufw.bin \
	rtlwifi/rtl8192defw.bin rtlwifi/rtl8192sefw.bin \
	rtlwifi/rtl8188efw.bin rtlwifi/rtl8188eufw.bin \
	rtlwifi/rtl8192cufw_A.bin \
	rtlwifi/rtl8192cufw_B.bin rtlwifi/rtl8192cufw_TMSC.bin \
	rtlwifi/rtl8192eefw.bin rtlwifi/rtl8192eu_ap_wowlan.bin \
	rtlwifi/rtl8192eu_nic.bin rtlwifi/rtl8192eu_wowlan.bin
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.rtlwifi_firmware.txt
endif

# rtl87xx
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_RTL_87XX),y)
LINUX_FIRMWARE_FILES += \
	rtlwifi/rtl8712u.bin rtlwifi/rtl8723fw.bin \
	rtlwifi/rtl8723fw_B.bin rtlwifi/rtl8723befw.bin \
	rtlwifi/rtl8723aufw_A.bin rtlwifi/rtl8723aufw_B.bin \
	rtlwifi/rtl8723aufw_B_NoBT.bin rtlwifi/rtl8723befw.bin \
	rtlwifi/rtl8723bs_ap_wowlan.bin rtlwifi/rtl8723bs_bt.bin \
	rtlwifi/rtl8723bs_nic.bin rtlwifi/rtl8723bs_wowlan.bin \
	rtlwifi/rtl8723bu_ap_wowlan.bin rtlwifi/rtl8723bu_nic.bin \
	rtlwifi/rtl8723bu_wowlan.bin rtlwifi/rtl8723befw_36.bin
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.rtlwifi_firmware.txt
endif

# rtl88xx
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_RTL_88XX),y)
LINUX_FIRMWARE_FILES += \
	rtlwifi/rtl8821aefw.bin        \
	rtlwifi/rtl8821aefw_29.bin     \
	rtlwifi/rtl8821aefw_wowlan.bin \
	rtlwifi/rtl8821aefw_29.bin rtlwifi/rtl8822befw.bin
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.rtlwifi_firmware.txt
endif

# ar5523
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_ATHEROS_5523),y)
LINUX_FIRMWARE_FILES += ar5523.bin
# No license file; the license is in the file WHENCE
# which is installed unconditionally
endif

# ar6002
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_ATHEROS_6002),y)
LINUX_FIRMWARE_FILES += ath6k/AR6002
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.atheros_firmware
endif

# ar6003
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_ATHEROS_6003),y)
LINUX_FIRMWARE_FILES += ath6k/AR6003
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.atheros_firmware
endif

# ar6004
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_ATHEROS_6004),y)
LINUX_FIRMWARE_FILES += ath6k/AR6004
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.atheros_firmware
endif

# ar7010
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_ATHEROS_7010),y)
LINUX_FIRMWARE_FILES += ar7010.fw ar7010_1_1.fw htc_7010.fw ath9k_htc/htc_7010-1.4.0.fw
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.atheros_firmware
endif

# ar9170
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_ATHEROS_9170),y)
LINUX_FIRMWARE_FILES += ar9170-1.fw ar9170-2.fw carl9170-1.fw
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.atheros_firmware
endif

# ar9271
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_ATHEROS_9271),y)
LINUX_FIRMWARE_FILES += ar9271.fw htc_9271.fw ath9k_htc/htc_9271-1.4.0.fw
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.atheros_firmware
endif

# qca6174
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_ATHEROS_QCA6174),y)
LINUX_FIRMWARE_FILES += ath10k/QCA6174/hw2.1/board.bin      \
                        ath10k/QCA6174/hw2.1/board-2.bin    \
                        ath10k/QCA6174/hw2.1/firmware-5.bin \
                        ath10k/QCA6174/hw3.0/board.bin      \
                        ath10k/QCA6174/hw3.0/board-2.bin    \
                        ath10k/QCA6174/hw3.0/firmware-4.bin \
                        ath10k/QCA6174/hw3.0/firmware-6.bin
endif

# qca9377
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_ATHEROS_QCA9377),y)
LINUX_FIRMWARE_FILES += ath10k/QCA9377/hw1.0/board.bin      \
                        ath10k/QCA9377/hw1.0/firmware-5.bin \
                        ath10k/QCA9377/hw1.0/firmware-6.bin
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.atheros_firmware
endif

# qca988x
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_ATHEROS_QCA988X),y)
LINUX_FIRMWARE_FILES += ath10k/QCA9887/hw1.0/board.bin      \
                        ath10k/QCA9887/hw1.0/firmware-5.bin \
                        ath10k/QCA988X/hw2.0/board.bin      \
                        ath10k/QCA988X/hw2.0/firmware-4.bin \
                        ath10k/QCA988X/hw2.0/firmware-5.bin
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.atheros_firmware
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_ATH9K_HTC),y)
LINUX_FIRMWARE_FILES += ath9k_htc/htc_7010-1.4.0.fw \
                        ath9k_htc/htc_9271-1.4.0.fw
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.atheros_firmware
endif

# wil6210
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_ATHEROS_WIL6210), y)
LINUX_FIRMWARE_FILES += wil6210.fw \
                        wil6210.brd
endif
# No license file; the license is in the file WHENCE
# which is installed unconditionally

# keyspan
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_KEYSPAN),y)
LINUX_FIRMWARE_FILES += keyspan/mpr.fw keyspan/usa18x.fw \
                        keyspan/usa19.fw keyspan/usa19qi.fw \
                        keyspan/usa19qw.fw keyspan/usa19w.fw \
                        keyspan/usa28.fw keyspan/usa28xa.fw \
                        keyspan/usa28xb.fw keyspan/usa28x.fw \
                        keyspan/usa49w.fw keyspan/usa49wlc.fw \
                        keyspan_pda/keyspan_pda.fw \
                        keyspan_pda/xircom_pgs.fw
# No license file; the license is in the file WHENCE
# which is installed unconditionally
endif

# libertas_tf_usb
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_LIBERTAS_8388),y)
LINUX_FIRMWARE_FILES += lbtf_usb.bin
# No license file; the license is in the file WHENCE
# which is installed unconditionally
endif

# sd8686 v8
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_LIBERTAS_SD8686_V8),y)
LINUX_FIRMWARE_FILES += libertas/sd8686_v8.bin libertas/sd8686_v8_helper.bin
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.Marvell
endif

# sd8686 v9
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_LIBERTAS_SD8686_V9),y)
LINUX_FIRMWARE_FILES += libertas/sd8686_v9.bin libertas/sd8686_v9_helper.bin
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.Marvell
endif

# sd8688
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_LIBERTAS_SD8688),y)
LINUX_FIRMWARE_FILES += mrvl/sd8688.bin mrvl/sd8688_helper.bin
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.Marvell
endif

# libertas_cs
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_LIBERTAS_CS),y)
LINUX_FIRMWARE_FILES +=  libertas/cf8385.bin        \
                         libertas/cf8385_helper.bin \
                         libertas/cf8381.bin        \
                         libertas/cf8381_helper.bin
# libertas_cs.fw is a symlink to libertas/cf8385.bin
# libertas_cs_helper.fw is a symlink to libertas/cf8385_helper.bin
define LINUX_FIRMWARE_LINK_LIBERTAS_CS
	ln -sf libertas/cf8385.bin $(TARGET_DIR)/lib/firmware/libertas_cs.fw
	ln -sf libertas/cf8385_helper.bin $(TARGET_DIR)/lib/firmware/libertas_cs_helper.fw
endef
LINUX_FIRMWARE_POST_INSTALL_TARGET_HOOKS += LINUX_FIRMWARE_LINK_LIBERTAS_CS
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.Marvell
endif

# mwifiex_pcie and mwifiex_usb
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_MRVL),y)
LINUX_FIRMWARE_FILES += mrvl/pcie8897_uapsta.bin      \
                        mrvl/usb8766_uapsta.bin       \
                        mrvl/usb8797_uapsta.bin       \
                        mrvl/usb8801_uapsta.bin       \
                        mrvl/usb8897_uapsta.bin       \
                        mrvl/usbusb8997_combo_v4.bin
# No license file; the license is in the file WHENCE
# which is installed unconditionally
endif

# mwl8k
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_MWL8K),y)
LINUX_FIRMWARE_FILES += mwl8k/fmimage_8366_ap-3.fw \
                        mwl8k/fmimage_8366.fw      \
                        mwl8k/helper_8366.fw       \
                        mwl8k/fmimage_8687.fw      \
                        mwl8k/helper_8687.fw
# No license file; the license is in the file WHENCE
# which is installed unconditionally
endif

# sd8787
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_MWIFIEX_SD8787),y)
LINUX_FIRMWARE_FILES += mrvl/sd8787_uapsta.bin
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.Marvell
endif

# sd8797
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_MWIFIEX_SD8797),y)
LINUX_FIRMWARE_FILES += mrvl/sd8797_uapsta.bin
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.Marvell
endif

# usb8797
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_MWIFIEX_USB8797),y)
LINUX_FIRMWARE_FILES += mrvl/usb8797_uapsta.bin
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.Marvell
endif

# usb8801
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_MWIFIEX_USB8801),y)
LINUX_FIRMWARE_FILES += mrvl/usb8801_uapsta.bin
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.Marvell
endif

# sd8887
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_MWIFIEX_SD8887),y)
LINUX_FIRMWARE_FILES += mrvl/sd8887_uapsta.bin
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.Marvell
endif

# sd8897
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_MWIFIEX_SD8897),y)
LINUX_FIRMWARE_FILES += mrvl/sd8897_uapsta.bin
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.Marvell
endif

# usb8897
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_MWIFIEX_USB8897),y)
LINUX_FIRMWARE_FILES += mrvl/usb8897_uapsta.bin
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.Marvell
endif

# pcie8897
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_MWIFIEX_PCIE8897),y)
LINUX_FIRMWARE_FILES += mrvl/pcie8897_uapsta.bin
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.Marvell
endif

# MT7601
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_MEDIATEK_MT7601U),y)
LINUX_FIRMWARE_FILES += mt7601u.bin
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.ralink_a_mediatek_company_firmware
endif

# orinoco
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_ORINOCO),y)
LINUX_FIRMWARE_FILES += agere_ap_fw.bin agere_sta_fw.bin
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.agere
endif

# qca6174
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_QUALCOMM_6174),y)
LINUX_FIRMWARE_FILES += ath10k/QCA6174
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENSE.QualcommAtheros_ath10k
endif

# wl127x
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_TI_WL127X),y)
LINUX_FIRMWARE_FILES += \
	ti-connectivity/wl1271-fw-2.bin \
	ti-connectivity/wl1271-fw-ap.bin \
	ti-connectivity/wl1271-fw.bin \
	ti-connectivity/wl127x-fw-3.bin \
	ti-connectivity/wl127x-fw-plt-3.bin \
	ti-connectivity/wl127x-nvs.bin \
	ti-connectivity/wl127x-fw-4-mr.bin \
	ti-connectivity/wl127x-fw-4-plt.bin \
	ti-connectivity/wl127x-fw-4-sr.bin \
	ti-connectivity/wl127x-fw-5-mr.bin \
	ti-connectivity/wl127x-fw-5-plt.bin \
	ti-connectivity/wl127x-fw-5-sr.bin \
	ti-connectivity/TIInit_7.2.31.bts
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.ti-connectivity
define LINUX_FIRMWARE_LINK_TI_WL127X
	ln -sf ti-connectivity/wl127x-nvs.bin $(TARGET_DIR)/lib/firmware/ti-connectivity/wl1271-nvs.bin
endef
LINUX_FIRMWARE_POST_INSTALL_TARGET_HOOKS += LINUX_FIRMWARE_LINK_TI_WL127X
endif

# wl128x
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_TI_WL128X),y)
LINUX_FIRMWARE_FILES += \
	ti-connectivity/wl128x-fw-3.bin \
	ti-connectivity/wl128x-fw-ap.bin \
	ti-connectivity/wl128x-fw-plt-3.bin \
	ti-connectivity/wl128x-fw.bin \
	ti-connectivity/wl128x-nvs.bin \
	ti-connectivity/wl127x-nvs.bin \
	ti-connectivity/wl128x-fw-4-mr.bin \
	ti-connectivity/wl128x-fw-4-plt.bin \
	ti-connectivity/wl128x-fw-4-sr.bin \
	ti-connectivity/wl128x-fw-5-mr.bin \
	ti-connectivity/wl128x-fw-5-plt.bin \
	ti-connectivity/wl128x-fw-5-sr.bin \
	ti-connectivity/TIInit_7.2.31.bts
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.ti-connectivity
define LINUX_FIRMWARE_LINK_TI_WL128X
	ln -sf ti-connectivity/wl127x-nvs.bin $(TARGET_DIR)/lib/firmware/ti-connectivity/wl1271-nvs.bin
	ln -sf ti-connectivity/wl127x-nvs.bin $(TARGET_DIR)/lib/firmware/ti-connectivity/wl12xx-nvs.bin
endef
LINUX_FIRMWARE_POST_INSTALL_TARGET_HOOKS += LINUX_FIRMWARE_LINK_TI_WL128X
endif

# wl18xx
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_TI_WL18XX),y)
LINUX_FIRMWARE_FILES += \
	ti-connectivity/wl18xx-fw.bin \
	ti-connectivity/wl18xx-fw-2.bin \
	ti-connectivity/wl18xx-fw-3.bin \
	ti-connectivity/wl18xx-fw-4.bin \
	ti-connectivity/wl127x-nvs.bin \
	ti-connectivity/TIInit_7.2.31.bts
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.ti-connectivity
define LINUX_FIRMWARE_LINK_TI_WL18XX
	ln -sf ti-connectivity/wl127x-nvs.bin $(TARGET_DIR)/lib/firmware/ti-connectivity/wl1271-nvs.bin
endef
LINUX_FIRMWARE_POST_INSTALL_TARGET_HOOKS += LINUX_FIRMWARE_LINK_TI_WL18XX
endif

# usb8xxx
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_LIBERTAS_USB8XXX),y)
LINUX_FIRMWARE_FILES += libertas/usb8682.bin \
                        libertas/usb8388_v5.bin \
                        libertas/usb8388_v9.bin
define LINUX_FIRMWARE_LINK_USB8388
	ln -sf libertas/usb8388_v5.bin $(TARGET_DIR)/lib/firmware/usb8388.bin
	ln -sf usb8388_v5.bin $(TARGET_DIR)/lib/firmware/libertas/usb8388.bin
endef
LINUX_FIRMWARE_POST_INSTALL_TARGET_HOOKS += LINUX_FIRMWARE_LINK_USB8388
# No license file; the license is in the file WHENCE
# which is installed unconditionally
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_IWLWIFI_100),y)
LINUX_FIRMWARE_FILES += iwlwifi-100-5.ucode
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.iwlwifi_firmware
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_IWLWIFI_105),y)
LINUX_FIRMWARE_FILES += iwlwifi-105-6.ucode
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.iwlwifi_firmware
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_IWLWIFI_135),y)
LINUX_FIRMWARE_FILES += iwlwifi-135-6.ucode
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.iwlwifi_firmware
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_IWLWIFI_1000),y)
LINUX_FIRMWARE_FILES += iwlwifi-1000-5.ucode
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.iwlwifi_firmware
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_IWLWIFI_2200),y)
LINUX_FIRMWARE_FILES += iwlwifi-2000-6.ucode
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.iwlwifi_firmware
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_IWLWIFI_2230),y)
LINUX_FIRMWARE_FILES += iwlwifi-2030-6.ucode
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.iwlwifi_firmware
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_IWLWIFI_3160),y)
LINUX_FIRMWARE_FILES += iwlwifi-3160-*.ucode
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.iwlwifi_firmware
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_IWLWIFI_3168),y)
LINUX_FIRMWARE_FILES += iwlwifi-3168-*.ucode
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.iwlwifi_firmware
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_IWLWIFI_3945),y)
LINUX_FIRMWARE_FILES += iwlwifi-3945-2.ucode
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.iwlwifi_firmware
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_IWLWIFI_4965),y)
LINUX_FIRMWARE_FILES += iwlwifi-4965-2.ucode
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.iwlwifi_firmware
endif

# iwlwifi 5000. Multiple files are available (iwlwifi-5000-1.ucode,
# iwlwifi-5000-2.ucode, iwlwifi-5000-5.ucode), corresponding to
# different versions of the firmware API. For now, we only install the
# most recent one.
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_IWLWIFI_5000),y)
LINUX_FIRMWARE_FILES += iwlwifi-5000-5.ucode
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.iwlwifi_firmware
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_IWLWIFI_5150),y)
LINUX_FIRMWARE_FILES += iwlwifi-5150-2.ucode
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.iwlwifi_firmware
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_IWLWIFI_6000),y)
LINUX_FIRMWARE_FILES += iwlwifi-6000-4.ucode
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.iwlwifi_firmware
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_IWLWIFI_6000G2A),y)
LINUX_FIRMWARE_FILES += iwlwifi-6000g2a-5.ucode \
                        iwlwifi-6000g2a-6.ucode
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.iwlwifi_firmware
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_IWLWIFI_6000G2B),y)
LINUX_FIRMWARE_FILES += iwlwifi-6000g2b-*.ucode
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.iwlwifi_firmware
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_IWLWIFI_6250),y)
LINUX_FIRMWARE_FILES += iwlwifi-6050-4.ucode \
                        iwlwifi-6050-5.ucode
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.iwlwifi_firmware
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_IWLWIFI_7260),y)
LINUX_FIRMWARE_FILES += iwlwifi-7260-*.ucode
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.iwlwifi_firmware
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_IWLWIFI_7265),y)
LINUX_FIRMWARE_FILES += iwlwifi-7265-*.ucode
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.iwlwifi_firmware
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_IWLWIFI_7265D),y)
LINUX_FIRMWARE_FILES += iwlwifi-7265D-*.ucode
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.iwlwifi_firmware
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_IWLWIFI_8000C),y)
LINUX_FIRMWARE_FILES += iwlwifi-8000C-*.ucode
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.iwlwifi_firmware
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_IWLWIFI_8265),y)
LINUX_FIRMWARE_FILES += iwlwifi-8265-*.ucode
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.iwlwifi_firmware
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_IWLWIFI_9XXX),y)
LINUX_FIRMWARE_FILES += iwlwifi-9???-*.ucode
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.iwlwifi_firmware
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_IWLWIFI_22000),y)
LINUX_FIRMWARE_FILES += iwlwifi-Qu-*.ucode \
                        iwlwifi-QuZ-*.ucode
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.iwlwifi_firmware
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_IWLWIFI_22260),y)
LINUX_FIRMWARE_FILES += iwlwifi-cc-*.ucode
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.iwlwifi_firmware
endif

# bnx2
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_BNX2),y)
LINUX_FIRMWARE_FILES += \
	bnx2/bnx2-mips-06-6.2.3.fw \
	bnx2/bnx2-mips-09-6.2.1b.fw \
	bnx2/bnx2-rv2p-06-6.0.15.fw \
	bnx2/bnx2-rv2p-09-6.0.17.fw \
	bnx2/bnx2-rv2p-09ax-6.0.17.fw
# No license file; the license is in the file WHENCE
# which is installed unconditionally
endif

# bnx2x
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_BNX2X),y)
LINUX_FIRMWARE_FILES += bnx2x/*
# No license file; the license is in the file WHENCE
# which is installed unconditionally
endif

# bna
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_BROCADE_BNA),y)
LINUX_FIRMWARE_FILES += ct2fw-3.2.3.0.bin \
                        ct2fw-3.2.5.1.bin \
                        ctfw-3.2.3.0.bin \
                        ctfw-3.2.5.1.bin
# No license file; the license is in the file WHENCE
# which is installed unconditionally
endif

# cxgb3
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_CXGB3),y)
LINUX_FIRMWARE_FILES += cxgb3/ael2005_opt_edc.bin \
                        cxgb3/ael2005_twx_edc.bin \
                        cxgb3/ael2020_twx_edc.bin \
                        cxgb3/t3b_psram-1.1.0.bin \
                        cxgb3/t3c_psram-1.1.0.bin \
                        cxgb3/t3fw-7.12.0.bin
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.chelsio_firmware
endif

# cxgb4
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_CXGB4_T4),y)
LINUX_FIRMWARE_FILES += cxgb4/t4fw-1.25.4.0.bin
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.chelsio_firmware
# cxgb4/t4fw.bin is a symlink to cxgb4/t4fw-1.25.4.0.bin
define LINUX_FIRMWARE_LINK_CXGB4_T4_FIRMWARE
       ln -sf t4fw-1.25.4.0.bin $(TARGET_DIR)/lib/firmware/cxgb4/t4fw.bin
endef
LINUX_FIRMWARE_POST_INSTALL_TARGET_HOOKS += LINUX_FIRMWARE_LINK_CXGB4_T4_FIRMWARE
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_CXGB4_T5),y)
LINUX_FIRMWARE_FILES += cxgb4/t5fw-1.25.4.0.bin
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.chelsio_firmware
# cxgb4/t5fw.bin is a symlink to cxgb4/t5fw-1.25.4.0.bin
define LINUX_FIRMWARE_LINK_CXGB4_T5_FIRMWARE
       ln -sf t5fw-1.25.4.0.bin $(TARGET_DIR)/lib/firmware/cxgb4/t5fw.bin
endef
LINUX_FIRMWARE_POST_INSTALL_TARGET_HOOKS += LINUX_FIRMWARE_LINK_CXGB4_T5_FIRMWARE
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_CXGB4_T6),y)
LINUX_FIRMWARE_FILES += cxgb4/t6fw-1.25.4.0.bin
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.chelsio_firmware
# cxgb4/t6fw.bin is a symlink to cxgb4/t6fw-1.25.4.0.bin
define LINUX_FIRMWARE_LINK_CXGB4_T6_FIRMWARE
       ln -sf t6fw-1.25.4.0.bin $(TARGET_DIR)/lib/firmware/cxgb4/t6fw.bin
endef
LINUX_FIRMWARE_POST_INSTALL_TARGET_HOOKS += LINUX_FIRMWARE_LINK_CXGB4_T6_FIRMWARE
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_KAWETH),y)
LINUX_FIRMWARE_FILES += kaweth/new_code.bin \
                        kaweth/new_code_fix.bin \
                        kaweth/trigger_code.bin \
                        kaweth/trigger_code_fix.bin
endif

# myri10ge
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_MYRICOM),y)
LINUX_FIRMWARE_FILES += myri10ge_eth_z8e.dat     \
                        myri10ge_ethp_z8e.dat    \
                        myri10ge_rss_eth_z8e.dat \
                        myri10ge_rss_ethp_z8e.dat
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.myri10ge_firmware
endif

# netxen_nic qlcnic
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_NETXEN_QLCNIC),y)
LINUX_FIRMWARE_FILES += phanfw.bin
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.phanfw
endif

# r8169
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_RTL_8169),y)
LINUX_FIRMWARE_FILES += rtl_nic/rtl8105e-1.fw \
                        rtl_nic/rtl8106e-1.fw \
                        rtl_nic/rtl8106e-2.fw \
                        rtl_nic/rtl8107e-1.fw \
                        rtl_nic/rtl8107e-2.fw \
                        rtl_nic/rtl8125a-3.fw \
                        rtl_nic/rtl8125b-1.fw \
                        rtl_nic/rtl8125b-2.fw \
                        rtl_nic/rtl8168d-1.fw \
                        rtl_nic/rtl8168d-2.fw \
                        rtl_nic/rtl8168e-1.fw \
                        rtl_nic/rtl8168e-2.fw \
                        rtl_nic/rtl8168e-3.fw \
                        rtl_nic/rtl8168f-1.fw \
                        rtl_nic/rtl8168f-2.fw \
                        rtl_nic/rtl8168fp-3.fw \
                        rtl_nic/rtl8168g-1.fw \
                        rtl_nic/rtl8168g-2.fw \
                        rtl_nic/rtl8168g-3.fw \
                        rtl_nic/rtl8168h-1.fw \
                        rtl_nic/rtl8168h-2.fw \
                        rtl_nic/rtl8402-1.fw \
                        rtl_nic/rtl8411-1.fw \
                        rtl_nic/rtl8411-2.fw
# No license file; the license is in the file WHENCE
# which is installed unconditionally
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_XC5000),y)
LINUX_FIRMWARE_FILES += dvb-fe-xc5000-1.6.114.fw
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.xc5000
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_XCx000),y)
LINUX_FIRMWARE_FILES += \
	dvb-fe-xc4000-1.4.1.fw \
	dvb-fe-xc5000-1.6.114.fw \
	dvb-fe-xc5000c-4.1.30.7.fw
LINUX_FIRMWARE_ALL_LICENSE_FILES += \
	LICENCE.xc4000 \
	LICENCE.xc5000 \
	LICENCE.xc5000c
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_AS102),y)
LINUX_FIRMWARE_FILES += as102_data1_st.hex as102_data2_st.hex
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.Abilis
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_DIB0700),y)
LINUX_FIRMWARE_FILES += dvb-usb-dib0700-1.20.fw
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENSE.dib0700
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_ITETECH_IT9135),y)
LINUX_FIRMWARE_FILES += dvb-usb-it9135-01.fw dvb-usb-it9135-02.fw
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.it913x
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_H5_DRXK),y)
LINUX_FIRMWARE_FILES += dvb-usb-terratec-h5-drxk.fw
# No license file; the license is in the file WHENCE
# which is installed unconditionally
endif

# brcm43xx
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_BRCM_BCM43XX),y)
LINUX_FIRMWARE_FILES += \
	brcm/bcm43xx-0.fw brcm/bcm43xx_hdr-0.fw \
	brcm/bcm4329-fullmac-4.bin brcm/brcmfmac4329-sdio.bin   \
	brcm/brcmfmac4330-sdio.bin brcm/brcmfmac4334-sdio.bin   \
	brcm/brcmfmac4335-sdio.bin brcm/brcmfmac4350-pcie.bin   \
	brcm/brcmfmac4350c2-pcie.bin brcm/brcmfmac4358-pcie.bin \
	brcm/brcmfmac4366b-pcie.bin brcm/brcmfmac4366c-pcie.bin \
	brcm/brcmfmac4371-pcie.bin brcm/brcmfmac4373.bin
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.broadcom_bcm43xx
endif

# brcm43xxx
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_BRCM_BCM43XXX),y)
LINUX_FIRMWARE_FILES += \
	brcm/brcmfmac43143.bin brcm/brcmfmac43143-sdio.bin \
	brcm/brcmfmac43236b.bin brcm/brcmfmac43241b0-sdio.bin \
	brcm/brcmfmac43241b4-sdio.bin brcm/brcmfmac43241b5-sdio.bin \
	brcm/brcmfmac43242a.bin brcm/brcmfmac43430a0-sdio.bin \
	brcm/brcmfmac43569.bin brcm/brcmfmac43602-pcie.ap.bin \
	brcm/brcmfmac43602-pcie.bin
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.broadcom_bcm43xx
endif

# broadcom tigon3
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_BROADCOM_TIGON3),y)
LINUX_FIRMWARE_FILES += tigon/tg3.bin \
	tigon/tg357766.bin \
	tigon/tg3_tso.bin \
	tigon/tg3_tso5.bin
endif

# i2400m-usb
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_INTEL_I2400),y)
LINUX_FIRMWARE_FILES += \
   i2400m-fw-usb-1.$(BR2_PACKAGE_LINUX_FIRMWARE_INTEL_I2400_REV).sbcf \
   i6050-fw-usb-1.5.sbcf
# No license file; the license is in the file WHENCE
# which is installed unconditionally
endif

# ql2xxx
ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_QLOGIC_2XXX),y)
LINUX_FIRMWARE_FILES += \
	ql2100_fw.bin ql2200_fw.bin ql2300_fw.bin ql2322_fw.bin \
	ql2400_fw.bin ql2500_fw.bin
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.qla2xxx
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_REDPINE_RS9113),y)
LINUX_FIRMWARE_FILES += rsi/rs9113_wlan_qspi.rps
# No license file; the license is in the file WHENCE
# which is installed unconditionally
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_QAT_DH895XCC),y)
# qat_mmp.bin is a symlink to qat_895xcc_mmp.bin
LINUX_FIRMWARE_FILES += qat_895xcc.bin qat_895xcc_mmp.bin qat_mmp.bin
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.qat_firmware
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_QAT_C3XXX),y)
LINUX_FIRMWARE_FILES += qat_c3xxx.bin qat_c3xxx_mmp.bin
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.qat_firmware
endif

ifeq ($(BR2_PACKAGE_LINUX_FIRMWARE_QAT_C62X),y)
LINUX_FIRMWARE_FILES += qat_c62x.bin qat_c62x_mmp.bin
LINUX_FIRMWARE_ALL_LICENSE_FILES += LICENCE.qat_firmware
endif

ifneq ($(LINUX_FIRMWARE_FILES),)
define LINUX_FIRMWARE_INSTALL_FILES
	cd $(@D) && \
		$(TAR) cf install.tar $(sort $(LINUX_FIRMWARE_FILES)) && \
		$(TAR) xf install.tar -C $(TARGET_DIR)/lib/firmware
endef
endif

ifneq ($(LINUX_FIRMWARE_DIRS),)
# We need to rm-rf the destination directory to avoid copying
# into it in itself, should we re-install the package.
define LINUX_FIRMWARE_INSTALL_DIRS
	$(foreach d,$(LINUX_FIRMWARE_DIRS), \
		rm -rf $(TARGET_DIR)/lib/firmware/$(d); \
		cp -a $(@D)/$(d) $(TARGET_DIR)/lib/firmware/$(d)$(sep))
endef
endif

ifneq ($(LINUX_FIRMWARE_FILES)$(LINUX_FIRMWARE_DIRS),)

# Most firmware files are under a proprietary license, so no need to
# repeat it for every selections above. Those firmwares that have more
# lax licensing terms may still add them on a per-case basis.
LINUX_FIRMWARE_LICENSE += Proprietary

# This file contains some licensing information about all the firmware
# files found in the linux-firmware package, so we always add it, even
# for firmwares that have their own licensing terms.
LINUX_FIRMWARE_ALL_LICENSE_FILES += WHENCE

# Some license files may be listed more than once, so we have to remove
# duplicates
LINUX_FIRMWARE_LICENSE_FILES = $(sort $(LINUX_FIRMWARE_ALL_LICENSE_FILES))

endif

define LINUX_FIRMWARE_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/lib/firmware
	$(LINUX_FIRMWARE_INSTALL_FILES)
	$(LINUX_FIRMWARE_INSTALL_DIRS)
endef

$(eval $(generic-package))
