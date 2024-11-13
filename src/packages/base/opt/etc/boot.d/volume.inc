#!/bin/sh
#----------------------------------------------------------------------------
# /etc/boot.d/volume.inc
# Functions for finding/mounting/unmounting volumes.
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

if [ "$volume_api" != yes ]
then

# List all block-devices excluding the raw device if partitions exist on it
scandevices()
{
    local major minor blocks name basedev lastdev
    sed '1,2d' /proc/partitions | while read major minor blocks name
    do
        echo "$name" | grep -q "^\(ram\|sr\)[0-9]\+$" && continue
        if [ -z "$basedev" ] || ! echo "$name" | grep -q "^$basedev"
        then
            [ -n "$lastdev" ] && echo $lastdev
            basedev=$name
            lastdev=$name
        else
            echo $name
            lastdev=
        fi
    done
}

# Checks an unmounted filesystem.
# $1 = device
# $2 = file system
check_fs()
{
    case $2 in
    vfat)
        if type -p fsck.fat >/dev/null
        then
            fsck.fat -c 850 -a "$1"
        fi
        ;;
    ext2|ext3|ext4)
        if type -p e2fsck >/dev/null
        then
            e2fsck -p -C 0 "$1"
        fi
        ;;
    *)
        return 0
        ;;
    esac
}

# Mounts a volume. Checks its file system if possible before mounting.
# $1 = device
# $2 = mount path
# $3 = file system (vfat, ext[234])
# $4 = mount options (rw, ro, ...)
mount_volume()
{
    local opt=$4
    [ -n "$opt" ] && opt="-o $opt"

    check_fs $1 $3
    local rc=$?
    if [ $rc -gt 1 ]
    then
        log_error "File system check failed for $1 (return code $rc)! Mount aborted!"
        return 1
    elif ! mount -t $3 $opt $1 "$2"
    then
        log_error "Unable to mount volume $1 on $2!"
        return 2
    else
        return 0
    fi
}

volume_api='yes'

fi # $volume_api != yes
