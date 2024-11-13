################################################################################
#
# atmel-firmware
#
################################################################################

ATMEL_FIRMWARE_VERSION = 1.3
ATMEL_FIRMWARE_SITE = http://www.thekelleys.org.uk/atmel
ATMEL_FIRMWARE_SOURCE = atmel-firmware-$(ATMEL_FIRMWARE_VERSION).tar.gz

ATMEL_FIRMWARE_FILES_PCI += images/atmel_at76c502.bin           \
                            images/atmel_at76c506.bin           \
                            images/atmel_at76c506-wpa.bin       \
                            images/atmel_at76c504a_2958-wpa.bin \
                            images/atmel_at76c504_2958-wpa.bin  \
                            images/atmel_at76c504.bin           \
                            images/atmel_at76c504c-wpa.bin      \
                            images/atmel_at76c502_3com.bin      \
                            images/atmel_at76c502_3com-wpa.bin  \
                            images/atmel_at76c502e.bin          \
                            images/atmel_at76c502e-wpa.bin      \
                            images/atmel_at76c502d.bin          \
                            images/atmel_at76c502d-wpa.bin      \
                            images/atmel_at76c502-wpa.bin
ATMEL_FIRMWARE_FILES_USB += images.usb/atmel_at76c505a-rfmd2958.bin \
                            images.usb/atmel_at76c505-rfmd2958.bin  \
                            images.usb/atmel_at76c505-rfmd.bin      \
                            images.usb/atmel_at76c503-rfmd-acc.bin  \
                            images.usb/atmel_at76c503-rfmd.bin      \
                            images.usb/atmel_at76c503-i3863.bin     \
                            images.usb/atmel_at76c503-i3861.bin
ATMEL_FIRMWARE_FILES += $(ATMEL_FIRMWARE_FILES_PCI) 
ATMEL_FIRMWARE_FILES += $(ATMEL_FIRMWARE_FILES_USB) 

ATMEL_FIRMWARE_LICENSE = Proprietary
ATMEL_FIRMWARE_LICENSE_FILES = COPYING

define ATMEL_FIRMWARE_INSTALL_TARGET_CMDS
   mkdir -p $(TARGET_DIR)/lib/firmware
   $(TAR) c -C $(@D)/images $(notdir $(ATMEL_FIRMWARE_FILES_PCI)) | \
      $(TAR) x -C $(TARGET_DIR)/lib/firmware
   $(TAR) c -C $(@D)/images.usb $(notdir $(ATMEL_FIRMWARE_FILES_USB)) | \
      $(TAR) x -C $(TARGET_DIR)/lib/firmware
	# handle copies of atmel_at76c504_2958-wpa.bin and atmel_at76c504a_2958-wpa.bin
	cp $(TARGET_DIR)/lib/firmware/atmel_at76c504_2958-wpa.bin $(TARGET_DIR)/lib/firmware/atmel_at76c504_2958.bin
	cp $(TARGET_DIR)/lib/firmware/atmel_at76c504a_2958-wpa.bin $(TARGET_DIR)/lib/firmware/atmel_at76c504a_2958.bin
	# rename atmel_at76c504c.bin according to
	# http://sources.gentoo.org/cgi-bin/viewvc.cgi/gentoo-x86/sys-firmware/atmel-firmware/atmel-firmware-1.3-r1.ebuild?view=diff&r1=text&tr1=1.1&r2=text&tr2=1.1&diff_format=s
	mv $(TARGET_DIR)/lib/firmware/atmel_at76c504c-wpa.bin $(TARGET_DIR)/lib/firmware/atmel_at76c504-wpa.bin

endef

$(eval $(generic-package))
