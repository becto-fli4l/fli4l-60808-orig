LCDHITACHI_VERSION = $(call qstrip,$(BR2_LINUX_KERNEL_VERSION))
LCDHITACHI_SITE = $(FLI4L_SRCDIR)/lcd/lcd/driver-rr
LCDHITACHI_SITE_METHOD = local
LCDHITACHI_SOURCE =

$(eval $(kernel-module))
$(eval $(generic-package))
