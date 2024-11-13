GENERIC_LEDS_VERSION = $(call qstrip,$(BR2_LINUX_KERNEL_VERSION))
GENERIC_LEDS_SITE = $(FLI4L_SRCDIR)/hwsupp/generic-leds
GENERIC_LEDS_SITE_METHOD = local
GENERIC_LEDS_SOURCE =

$(eval $(kernel-module))
$(eval $(generic-package))
