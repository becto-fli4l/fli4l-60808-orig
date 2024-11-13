#!/bin/sh
#------------------------------------------------------------------------------
# Boot loader specific code                                        __FLI4LVER__
#
# Last Update: $Id$
#------------------------------------------------------------------------------

#################################
### GENERIC BOILER-PLATE CODE ###
#################################

# $1 = architecture
# $2 = build directory
boot_loader_post_build()
{
    arch=$1
    shift
    case $arch in
    x86|x86_64|rpi|sunxi)
        boot_loader_post_build_${arch} "$@"
        ;;
    esac
}

# $1 = architecture
# $2 = build directory
boot_loader_get_entries()
{
    arch=$1
    shift
    case $arch in
    x86|x86_64|rpi|sunxi)
        boot_loader_get_entries_${arch} "$@"
        ;;
    esac
}

# $1 = architecture
# $2 = partition device
# $3 = block device
boot_loader_install()
{
    arch=$1
    shift
    case $arch in
    x86|x86_64|rpi|sunxi)
        boot_loader_install_${arch} "$@"
        ;;
    esac
}

###################################
### ARCHITECTURE DEPENDANT CODE ###
###################################

##################
### x86/x86_64 ###
##################

boot_loader_post_build_x86()
{
    boot_loader_post_build_x86_and_x86_64 "$@"
}

boot_loader_get_entries_x86()
{
    boot_loader_get_entries_x86_and_x86_64 "$@"
}

boot_loader_install_x86()
{
    boot_loader_install_x86_and_x86_64 "$@"
}

boot_loader_post_build_x86_64()
{
    boot_loader_post_build_x86_and_x86_64 "$@"
}

boot_loader_get_entries_x86_64()
{
    boot_loader_get_entries_x86_and_x86_64 "$@"
}

boot_loader_install_x86_64()
{
    boot_loader_install_x86_and_x86_64 "$@"
}

boot_loader_post_build_x86_and_x86_64()
{
    : # nothing to do
}

boot_loader_get_entries_x86_and_x86_64()
{
    echo "$1/syslinux.cfg syslinux.cfg 1"
}

boot_loader_install_x86_and_x86_64()
{
    part_dev=$1
    blk_dev=$2

    # write a fresh master boot record
    dd bs=440 count=1 conv=notrunc if=opt/usr/share/syslinux/mbr.bin of=$blk_dev

    # make partition bootable using fdisk
    # first we have to check whether the partition is already bootable
    part_number=$(echo "$part_dev" | sed -n 's,.*[^[:digit:]]\([[:digit:]]\+\)$,\1,p')
    bootable=$(fdisk -l "$blk_dev" | sed -n "s,^${part_dev}[[:space:]]*\*.*,y,p")

    if [ -z "$bootable" -a -n "$part_number" ]
    then
        #     a - toggle bootable flag
        #     w - write changes and quit
        /sbin/fdisk "$blk_dev" <<EOF
a
$part_number
w
q
EOF
    fi

    # install boot loader
    syslinux -i "$part_dev"
}

###########
### rpi ###
###########

boot_loader_post_build_rpi()
{
    # the RPi firmware prepares boot arguments for the kernel, we have to
    # include them before booting the kernel
    sed -i 's/^\([[:space:]]*APPEND \)/\1${bootargs} /' "$1"/syslinux.cfg

    mkdir -p "$1"/extlinux

    cat > "$1"/config.txt <<EOF
kernel=u-boot.bin
device_tree_address=0x100
gpu_mem=16
EOF

    # the RPi firmware expects the DTB files to lie in the boot medium's root
    # directory
    mv "$1"/dtbs/*.dtb "$1"/

    # copy the RPi boot loader files
    for f in bootcode.bin start.elf fixup.dat
    do
        cp -a opt/img/rpi-firmware/$f "$1"/
    done
    cp -a opt/img/u-boot.bin "$1"/
}

boot_loader_get_entries_rpi()
{
    # As wie use gpu_mem=16 to maximize the memory available for Linux kernel
    # and applications, the bootcode.bin searches for start_cd.elf and
    # fixup_cd.elf ("cd" standing for "cut-down"). So we have to rename the
    # files appropriately, as the FBR rpi-firmware package always uses the
    # names start.elf and fixup.dat.
    cat <<EOF
$1/syslinux.cfg extlinux/extlinux.conf 1
$1/config.txt config.txt 1
$1/bootcode.bin bootcode.bin 1
$1/start.elf start_cd.elf 1
$1/fixup.dat fixup_cd.dat 1
$1/u-boot.bin u-boot.bin 1
EOF

    for dtb in "$1"/*.dtb
    do
        echo "$dtb $(basename "$dtb") 1"
    done
}

boot_loader_install_rpi()
{
    : # nothing to do
}

#############
### sunxi ###
#############

boot_loader_post_build_sunxi()
{
    mkdir -p "$1/extlinux"
}

boot_loader_get_entries_sunxi()
{
    echo "$1/syslinux.cfg extlinux/extlinux.conf 1"
}

boot_loader_install_sunxi()
{
    part_dev=$1
    blk_dev=$2

    # check if there is place for the boot loader
    set -f
    set -- $(/sbin/fdisk -l $blk_dev | grep "^$part_dev ")
    set +f
    if [ -z "$1" ]
    then
        echo "Error: Boot volume $part_dev not found in partition table on $blk_dev!"
    elif [ $3 -lt 2048 ]
    then
        echo "Error: U-Boot needs 1 MiB room between start of device and start of boot volume, you only have $3 kiB!"
    else
        # install boot loader
        dd of=$blk_dev bs=1024 count=1023 seek=1 if=/dev/zero
        dd of=$blk_dev bs=1024 seek=8 if=opt/img/u-boot-sunxi-with-spl.bin
    fi
}
