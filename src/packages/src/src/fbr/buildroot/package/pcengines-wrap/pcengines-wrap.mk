PCENGINES_WRAP_VERSION = $(call qstrip,$(BR2_LINUX_KERNEL_VERSION))
PCENGINES_WRAP_SITE = $(FLI4L_SRCDIR)/hwsupp/pcengines-wrap
PCENGINES_WRAP_SITE_METHOD = local
PCENGINES_WRAP_SOURCE =

$(eval $(kernel-module))
$(eval $(generic-package))
