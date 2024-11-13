################################################################################
#
# syslinux to make target msdos/iso9660 filesystems bootable
#
################################################################################

SYSLINUX_VERSION = 6.03
SYSLINUX_SOURCE = syslinux-$(SYSLINUX_VERSION).tar.xz
SYSLINUX_SITE = $(BR2_KERNEL_MIRROR)/linux/utils/boot/syslinux

SYSLINUX_LICENSE = GPL-2.0+
SYSLINUX_LICENSE_FILES = COPYING

SYSLINUX_INSTALL_IMAGES = YES

# host-util-linux needed to provide libuuid when building host tools
SYSLINUX_DEPENDENCIES = host-nasm host-upx util-linux host-petools host-syslinux
HOST_SYSLINUX_DEPENDENCIES = host-nasm host-upx host-util-linux host-petools

# explicitly use BFD linker if available
# this is necessary if the host uses gold linker by default, because syslinux
# cannot be currently linked with gold
ifeq ($(shell ls $(HOSTLD).bfd 2>/dev/null),)
SYSLINUX_HOSTLD := $(HOSTLD)
else
SYSLINUX_HOSTLD := $(HOSTLD).bfd
endif

# --no-dynamic-linker is needed when it is available
ifneq ($(shell $(SYSLINUX_HOSTLD) --help 2>&1 | grep -- "--no-dynamic-linker"),)
SYSLINUX_LDLINUX_LDFLAGS := --no-dynamic-linker
else
SYSLINUX_LDLINUX_LDFLAGS :=
endif

# used for generated Windows binaries
SYSLINUX_TIMESTAMP = $(shell date +%s -d "2015-03-03 $(subst .,:,$(SYSLINUX_VERSION))")

ifeq ($(BR2_TARGET_SYSLINUX_LEGACY_BIOS),y)
SYSLINUX_TARGET += bios
endif

# The syslinux build system must be forced to use Buildroot's gnu-efi
# package by setting EFIINC, LIBDIR and LIBEFI. Otherwise, it uses its
# own copy of gnu-efi included in syslinux's sources since 6.03
# release.
ifeq ($(BR2_TARGET_SYSLINUX_EFI),y)
ifeq ($(BR2_ARCH_IS_64),y)
SYSLINUX_EFI_BITS = efi64
else
SYSLINUX_EFI_BITS = efi32
endif # 64-bit
SYSLINUX_DEPENDENCIES += gnu-efi
SYSLINUX_TARGET += $(SYSLINUX_EFI_BITS)
SYSLINUX_EFI_ARGS = \
	EFIINC=$(STAGING_DIR)/usr/include/efi \
	LIBDIR=$(STAGING_DIR)/usr/lib \
	LIBEFI=$(STAGING_DIR)/usr/lib/libefi.a
endif # EFI

define SYSLINUX_CDEMUL
	mkdir $(@D)/cdemul
	cp -a $(@D)/mtools/syslinux.c $(@D)/cdemul
	cp -a $(@D)/mtools/Makefile $(@D)/cdemul
endef
SYSLINUX_PRE_PATCH_HOOKS += SYSLINUX_CDEMUL
HOST_SYSLINUX_PRE_PATCH_HOOKS += SYSLINUX_CDEMUL

# The syslinux tarball comes with pre-compiled binaries.
# Since timestamps might not be in the correct order, a rebuild is
# not always triggered for all the different images.
# Cleanup the mess even before we attempt a build, so we indeed
# build everything from source.
define SYSLINUX_CLEANUP
	cp $(@D)/bios/win32/syslinux.exe $(@D)/win32
	cp $(@D)/bios/win64/syslinux64.exe $(@D)/win64
	rm -rf $(@D)/bios $(@D)/efi32 $(@D)/efi64
endef
SYSLINUX_POST_PATCH_HOOKS += SYSLINUX_CLEANUP

# syslinux build system has no convenient way to pass CFLAGS,
# and the internal zlib should take precedence so -I shouldn't
# be used.
# kristov: note that TARGET_LD always supports --no-dynamic-linker, so
# no run-time test is necessary like for HOSTLD.
define SYSLINUX_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE1) \
		CC="$(TARGET_CC)" \
		LD="$(TARGET_LD)" \
		SYSLINUX_LDLINUX_LDFLAGS="--no-dynamic-linker" \
		NASM="$(HOST_DIR)/bin/nasm" \
		CC_FOR_BUILD="$(HOSTCC)" \
		CFLAGS_FOR_BUILD="$(HOST_CFLAGS)" \
		LDFLAGS_FOR_BUILD="$(HOST_LDFLAGS)" \
		$(SYSLINUX_EFI_ARGS) -C $(@D) $(SYSLINUX_TARGET)
endef

define HOST_SYSLINUX_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE1) \
		CC="$(HOSTCC)" \
		LD="$(SYSLINUX_HOSTLD)" \
		EXTRA_INCLUDES="$(HOST_CPPFLAGS)" \
		SYSLINUX_LDLINUX_LDFLAGS="$(SYSLINUX_LDLINUX_LDFLAGS)" \
		NASM="$(HOST_DIR)/bin/nasm" \
		CC_FOR_BUILD="$(HOSTCC)" \
		CFLAGS_FOR_BUILD="$(HOST_CFLAGS)" \
		LDFLAGS_FOR_BUILD="$(HOST_LDFLAGS)" \
		$(SYSLINUX_EFI_ARGS) -C $(@D) $(SYSLINUX_TARGET)
endef

define SYSLINUX_INSTALL_TARGET_CMDS
	for i in $(SYSLINUX_IMAGES-y); do \
		$(INSTALL) -D -m 0755 $(@D)/$$i $(TARGET_DIR)/usr/share/syslinux/$${i##*/}; \
	done
	for i in $(SYSLINUX_C32); do \
		$(INSTALL) -D -m 0755 $(HOST_DIR)/share/syslinux/$${i} \
				   $(TARGET_DIR)/usr/share/syslinux/$${i}; \
	done
	# syslinux binary to be used on the fli4l (OPT_HDINSTALL)
	$(INSTALL) -D -m 755 $(@D)/bios/linux/syslinux $(TARGET_DIR)/usr/sbin/syslinux
endef

define HOST_SYSLINUX_INSTALL_CMDS
	$(TARGET_MAKE_ENV) $(MAKE1) $(SYSLINUX_EFI_ARGS) INSTALLROOT=$(HOST_DIR) \
		BINDIR="/bin" LIBDIR="/lib" DATADIR="/share" MANDIR="/man" \
		CC="$(HOSTCC)" \
		LD="$(SYSLINUX_HOSTLD)" \
		-C $(@D) $(SYSLINUX_TARGET) install
	# for img-cdemul package
	$(INSTALL) -D -m 755 $(@D)/bios/cdemul/syslinux $(HOST_DIR)/bin/syslinux-cdemul
endef

SYSLINUX_IMAGES-$(BR2_TARGET_SYSLINUX_ISOLINUX) += bios/core/isolinux.bin
SYSLINUX_IMAGES-$(BR2_TARGET_SYSLINUX_PXELINUX) += bios/core/pxelinux.bin
SYSLINUX_IMAGES-$(BR2_TARGET_SYSLINUX_MBR) += bios/mbr/mbr.bin
SYSLINUX_IMAGES-$(BR2_TARGET_SYSLINUX_EFI) += $(SYSLINUX_EFI_BITS)/efi/syslinux.efi

SYSLINUX_C32 = $(call qstrip,$(BR2_TARGET_SYSLINUX_C32))

# We install the c32 modules from the host-installed tree, where they
# are all neatly installed in a single location, while they are
# scattered around everywhere in the build tree.
define SYSLINUX_INSTALL_IMAGES_CMDS
	for i in $(SYSLINUX_IMAGES-y); do \
		$(INSTALL) -D -m 0755 $(@D)/$$i $(BINARIES_DIR)/syslinux/$${i##*/}; \
	done
	for i in $(SYSLINUX_C32); do \
		$(INSTALL) -D -m 0755 $(HOST_DIR)/share/syslinux/$${i} \
			$(BINARIES_DIR)/syslinux/$${i}; \
	done
	# syslinux binaries to be used on the host (mkhdinstall.{sh,au3})
	$(INSTALL) -D -m 644 $(@D)/bios/dos/syslinux.com $(BINARIES_DIR)/syslinux/syslinux.com
	$(INSTALL) -D -m 644 $(@D)/win32/syslinux.exe $(BINARIES_DIR)/syslinux/syslinux.exe
	$(INSTALL) -D -m 644 $(@D)/win64/syslinux64.exe $(BINARIES_DIR)/syslinux/syslinux64.exe
	$(HOST_DIR)/bin/petsset $(BINARIES_DIR)/syslinux/syslinux.exe $(SYSLINUX_TIMESTAMP)
endef

$(eval $(generic-package))
$(eval $(host-generic-package))

# when installing images, target files are needed
$(SYSLINUX_TARGET_INSTALL_IMAGES) : $(SYSLINUX_TARGET_INSTALL_TARGET)
