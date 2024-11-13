USBIP_VERSION     = 1.1.1
USBIP_SITE        = $(LINUX_HEADERS_DIR)/drivers/staging/usbip/userspace
USBIP_SITE_METHOD = local
USBIP_SOURCE      =
USBIP_AUTORECONF  = YES
USBIP_DEPENDENCIES = libglib2 libsysfs linux-headers

USBIP_CONF_ENV = CFLAGS="$(TARGET_CFLAGS) -Wno-error"

$(eval $(autotools-package))

$(USBIP_TARGET_RSYNC): $(LINUX_HEADERS_TARGET_PATCH)
