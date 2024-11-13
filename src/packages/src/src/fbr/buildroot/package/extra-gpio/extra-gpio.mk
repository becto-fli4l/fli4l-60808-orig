EXTRA_GPIO_VERSION = $(call qstrip,$(BR2_LINUX_KERNEL_VERSION))
EXTRA_GPIO_SITE = $(FLI4L_SRCDIR)/hwsupp/extra-gpio
EXTRA_GPIO_SITE_METHOD = local
EXTRA_GPIO_SOURCE =

$(eval $(kernel-module))
$(eval $(generic-package))
