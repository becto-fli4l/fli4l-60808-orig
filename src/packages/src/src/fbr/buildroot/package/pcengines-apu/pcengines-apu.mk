PCENGINES_APU_VERSION = $(call qstrip,$(BR2_LINUX_KERNEL_VERSION))
PCENGINES_APU_SITE = $(FLI4L_SRCDIR)/hwsupp/pcengines-apu
PCENGINES_APU_SITE_METHOD = local
PCENGINES_APU_SOURCE =

$(eval $(kernel-module))
$(eval $(generic-package))
