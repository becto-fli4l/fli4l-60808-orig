MO2IPC_A78_VERSION =
MO2IPC_A78_SITE = $(FLI4L_SRCDIR)/lcd/isdn_rate/mo2ipc_a78
MO2IPC_A78_SITE_METHOD = local
MO2IPC_A78_SOURCE =
MO2IPC_A78_DEPENDENCIES = host-nasm

define MO2IPC_A78_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) LD=$(TARGET_LD) \
		LDFLAGS="-m elf_i386 $(patsubst -Wl$(comma)%,%,$(TARGET_LDFLAGS))" \
		-C $(@D) mo2ipc_a78
endef

define MO2IPC_A78_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/mo2ipc_a78 $(TARGET_DIR)/usr/local/bin/mo2ipc_a78
endef

$(eval $(generic-package))

# we need to lie as Buildroot requires the architectures to match...
$(MO2IPC_A78_TARGET_INSTALL_TARGET): BR2_READELF_ARCH_NAME="Intel 80386"
