#!/bin/sh
#------------------------------------------------------------------------------
# Creates a fli4l-Floppy
#
# Created:        hansmi, 2004-03-31
# Last Update:    $Id$
#------------------------------------------------------------------------------

suffix=''                                       # fast but not safe
syslinux=syslinux.cfg       # default cfg
kernel='kernel'

while [ 1 ]
do
    echo $1
    case "$1"
    in
    -s)                                 # slow stupid safe
        suffix='-s'
        shift
        ;;
    -?)
        echo "usage: $0 [-s] [config-directory]" >&2;
        exit 1
        ;;
    *)
        break
        ;;
    esac
done

if [ "$1" != "" ]
then
    config_dir="$1"
else
    config_dir=config
fi

if ! sh ./mkopt.sh "$config_dir"
then
    exit 1
fi

cd unix/darwin
make FindDrive
cd ../..

finddrive='unix/darwin/FindDrive --header --removable --writable --ejectable --whole --name'
tempfile="`tempfile`"

echo "Executing $finddrive"
$finddrive > "$tempfile"

echo
echo 'Please select a floppy-drive:'

trap 'echo; rm "$tempfile"; exit' 2

floppy=
while [ -z "$floppy" ]
do
    count=0
    while read line
    do
        case "$line" in
            /dev/*) set -- $line
                count="`expr $count + 1`"
                eval "drive$count='$1'"
                line="[$count] $line"
            ;;
            *)
                line="    $line"
            ;;
        esac
        echo "$line"
    done < "$tempfile"
    if [ "$count" = 0 ]
    then
        echo 'No valid drives found... exiting'
        exit
    fi
    if [ "$count" != 1 ]
    then
        echo '[0] Exit'
        echo -n 'Number: '
        read input
    else
        echo 'Only one drive found, using this one.'
        input=1
    fi
    if [ "$input" = 0 ]
    then
        echo 'Exiting...'
        exit
    else
        eval cur='$drive'"$input"
        if [ -z "$cur" ]
        then
            floppy=
            echo 'Invalid drive... please select another one'
        else
            floppy="$cur"
        fi
    fi
done

rm -f "$tempfile"

echo "Writing to $floppy, this may DESTROY the data on it!"
echo -n 'Do you want to continue? [y/n] '
read answer
if [ "$answer" != y -a "$answer" != "yes" ]
then
    exit
fi

cd img

set -e

fat_image_file=fd1440$suffix.gz
mtoolsopts="-i $floppy"

echo "Unmounting $floppy..."
/usr/bin/hdiutil unmount $floppy

echo 'Writing FAT and system files...'
gzip -d <$fat_image_file >$floppy

echo 'Copying syslinux.cfg...'
mcopy $mtoolsopts -o $syslinux ::SYSLINUX.CFG

echo 'Copying kernel...'
mcopy $mtoolsopts -o $kernel ::KERNEL

echo 'Copying rootfs.tgz...'
mcopy $mtoolsopts -o rootfs.tgz ::ROOTFS.TGZ

echo 'Copying rc.cfg...'
mcopy $mtoolsopts -o rc.cfg ::RC.CFG

echo 'Copying opt.tar.bz2...'
mcopy $mtoolsopts -o opt_tar.bz2 ::OPT_TAR.BZ2

echo "Ejecting $floppy... please insert it again to mount it"
/usr/bin/hdiutil eject $floppy

