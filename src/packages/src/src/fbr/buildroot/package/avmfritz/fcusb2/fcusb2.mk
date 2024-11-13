FCUSB2_CARD          := fcusb2
FCUSB2_AVMTYPE       := km_fritzcapi
ifeq (y,$(BR2_ARCH_IS_64))
FCUSB2_SUBDIR_DRIVER := fritzcapi/fritz.usb2/src
else
FCUSB2_SUBDIR_DRIVER := fritzcapi/fritz.usb/src
endif
FCUSB2_SUBDIR_LIB    := fritzcapi/fritz.usb2/lib

include package/avmfritz/avmfritz.inc
