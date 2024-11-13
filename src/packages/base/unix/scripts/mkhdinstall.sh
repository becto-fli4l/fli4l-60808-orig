#! /bin/sh
# -------------------------------------------------------------------------
# creates a script to do a direct-hd-install                   __FLI4LVER__
#
# Creation:       2008-02-24 jb / LanSpezi
# Last Update:    $Id$
# -------------------------------------------------------------------------

scr_mounted=

example()
{
    cat <<EOF
Either add

    <device> <mount point> vfat rw,user,noauto,umask=000 0 0

to /etc/fstab or mount the device before invoking mkfli4l.sh,
for instance  like follows

    [sudo] mount <device> <mount point> -t vfat -o umask=000

or as complete sequence:

    sudo mount <device> <mount point> -t vfat -o umask=000 && \
    sh mkfli4l.sh --hdinstallpath <mount point> ; \
    sudo umount <mount point>

EOF
}
check_mount()
{
    base_device_name=
    line=`mount | grep "on ${1%/}"`
    if [ "$line" ]; then
        set $line
        real_device=$1
        real_device_path=$(dirname $real_device)
        real_device_name=$(basename $real_device)
        fs=$5

        # most partition device names are of the form <base><number>, but if
        # the base device ends with a number itself, the partition is called
        # <base><number>p<number>
        if echo $real_device_name | grep -q "^.*[[:digit:]]\+p[[:digit:]]\+$"; then
            base_device_name=$(echo $real_device_name | sed "s/^\(.*[[:digit:]]\)\+p[[:digit:]]\+$/\1/")
        else
            base_device_name=$(echo $real_device_name | sed -n "s/^\(.*\)[[:digit:]]\+$/\1/p")
        fi
        return 0
    fi
    return 1
}

mkhdinstall ()
{
    if ! check_mount $hdinstall_path; then
        # try to mount it as normal user
        scr_mounted=yes
        echo "-> mounting $hdinstall_path ..."
        if ! mount $hdinstall_path 2> /dev/null || ! check_mount $hdinstall_path; then
            # still not mounted, so no user entry in /etc/fstab
            {
                cat <<EOF
Unable to mount usb device, please either mount it before invoking
mkfli4l.sh or add an entry in /etc/fstab allowing us to mount the device.
EOF
                example
            } | log_error
            abort
        fi
    fi
    if [ "$fs" != vfat ]; then
        abort "Wrong file system on device $real_device, we expect 'vfat', but it is '$fs'."
    fi
    if [ "`cat /sys/block/$base_device_name/removable`" != "1" ]; then
        if [ -z "`ls -l /sys/dev/block/ | grep "/$base_device_name$" | grep usb`" ]; then
            echo "'$base_device_name' does not seem to be a removable device."

            answer="NO"
            if [ "$bool_batch" = "false" ]; then
                echo "If you really know what you're doing type YES"
                read answer
            elif [ "$bool_yes" = "true" ]; then
                answer="YES"
            fi
            if [ "$answer" != "YES" ]; then
                abort "Build aborted as '$device' does not seem to be a removable device."
            fi
        fi
    fi
    if [ ! -d $hdinstall_path -o ! -w  $hdinstall_path -o ! -x  $hdinstall_path ]; then
        {
            echo "$hdinstall_path not writable for us, please specify umask=000 as option to mount"
            example
        } | log_error
        abort
    fi

    # ready to go, device mounted, usb device, correct file system, writable
    cf_errors=0
    if ! echo "hd_boot=sda1" > $hdinstall_path/hd.cfg; then
        cf_errors=1
    else
        for target in $targets; do
            mkdir -p "$hdinstall_path/$(dirname $target)"
            echo -n "   $target"
            [ -d "$hdinstall_path/$target" ] && rm -rf "$hdinstall_path/$target"
            if cp -r "$dir_image/$target" "$hdinstall_path/$target"; then
                echo " [OK]"
            else
                echo " [!!]"
                cf_errors=1
            fi
        done
    fi

    if [ "$scr_mounted" = yes ]; then
        echo "-> unmounting $hdinstall_path"
        umount $hdinstall_path || abort "failed to unmount $hdinstall_path"
    fi
    [ $cf_errors -eq 0 ] || abort "Something went wrong. Get help."
}

show_header "--- trying to copy files to install medium"
mkhdinstall
show_end

if [ -n "$base_device_name" -a -b "/dev/$base_device_name" ]
then
    show_header "--- installing boot loader on /dev/$base_device_name"

    answer="NO"
    if [ "$bool_batch" = "false" ]; then
        echo "If you really know what you're doing type YES"
        read answer
    elif [ "$bool_yes" = "true" ]; then
        answer="YES"
    fi
    if [ "$answer" != "YES" ]
        then abort "Build aborted as boot loader could not be installed."
    fi

    boot_loader_install $arch $real_device /dev/$base_device_name
    show_end
else
    echo "Cannot install boot loader as base device \"$base_device_name\" is unknown or does not exist"
fi

if [ "$scr_mounted" != yes ]; then
    echo -e "\nDo not forget to 'umount $hdinstall_path' before removing the medium\n"
fi
echo -e 'Insert "disk" into your router and start.\nGood luck.\n'
