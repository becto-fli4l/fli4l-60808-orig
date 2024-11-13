#!/bin/sh
#----------------------------------------------------------------------------
# /usr/local/bin/hdinstall.sh - install router-disk on harddisk
#
# Creation:	    24.09.2003  hs
# Last Update:  $Id$
#----------------------------------------------------------------------------

boot_file_list="rc.cfg kernel rootfs.img opt.img opt.old syslinux.cfg boot.msg boot_r.msg boot_s.msg boot_z.msg persistent"
boot_file_list2="rc2.cfg kernel2 rootfs2.img opt2.img"

inv_partition ()
{
    # simulate fdisk -l output
    if true; then
	fdisk -l /dev/"$targetdisk"
    else
    cat <<EOF
Disk /dev/sda: 2048 MB, 2147483648 bytes, 4194304 sectors
16384 cylinders, 16 heads, 16 sectors/track
Units: cylinders of 256 * 512 = 131072 bytes

Device  Boot StartCHS    EndCHS        StartLBA     EndLBA    Sectors  Size Id Type
/dev/sda1 *  0,1,1       3,15,16             16       1023       1008  504K  6 FAT16
/dev/sda2 *  4,0,1       62,15,16          1024      16127      15104 7552K 83 Linux
EOF
    fi
}

check_partition()
{
    inv_partition > /tmp/ptable.$$
    if grep -q "Partition.*has different physical/logical endings" /tmp/ptable.$$; then
	$msg inv_partition_table
	read all_ok
	case "$all_ok" in
	    JA | YES | OUI) ;;
	    *)        exit ;;
	esac
	ptable_modified=yes
	cat <<EOF

#
# Old partition table
#

EOF
	cat /tmp/ptable.$$
	dd if=/dev/zero of=/dev/"$targetdisk" bs=512 count=2 >/dev/null 2>/dev/null
	dd if=/usr/share/syslinux/mbr.bin of=/dev/"$targetdisk" >/dev/null 2>/dev/null
	cat <<EOF | fdisk /dev/"$targetdisk" > /dev/null
n
p
1


w
q
EOF
	cat <<EOF

#
# New partition table
#
EOF
        fdisk -l /dev/"$targetdisk"
	echo
	echo "Press enter to continue"
	read all_ok
    fi
}
part_type_linux=83
part_type_linux_swap=82
part_type_fat12=1
part_type_fat16_max_32=4
part_type_fat16=6

tmpdir=/tmp/bootfiles
err_prefix="ERROR: "

cleanup ()
{
    rm -rf $tmpdir
    rm -f /tmp/newpart-$$ /tmp/hdconfig-$$ /tmp/ptable.$$ /tmp/fdisk.$$
}
error () # message [ exit ]
{
    colecho "$err_prefix$1" br x br
    if [ "$2" ]; then
	colecho "terminating installation ..." br x br
	cleanup
	exit 1
    fi
    err_prefix=
}

read_num ()
{
    read val
    case "$val" in
	X|x)
            exit
	    ;;
	"")
	    val=0
	    ;;
        [0-9]*)
	    if ! echo $val | grep -q -e '^[0-9]*$'; then
		error "Please enter either a number or X to exit!" exit
	    fi
	    ;;
	*)  error "Please enter either a number or X to exit!" exit ;;
    esac
}

add_partition () # start end type [ activate ]
{
    {
	cat <<EOF
n
p
$partnum
$1
$2
EOF
       case $partnum in
	   1) echo -e "t\n$3" ;;
	   *) echo -e "t\n$partnum\n$3" ;;
       esac
       if [ "$4" ]; then
	   echo -e "a\n$partnum"
       fi
    } >>/tmp/newpart-$$
}

get_free_space ()
{
    # get free size of boot-partition
    set `df $1 | grep -v ^Filesystem`
    free=$4
}

get_file_size ()
{
    du -c $1 | tail -n 1 | sed 's/^\([0-9]\+\).*/\1/'
}


get_boot_size ()
{
    files="$*"
    needed=0
    error=

    for i in $files; do
	if [ -f /boot/$i -o -d /boot/$i ]; then
	    size=$(get_file_size /boot/$i)
	    needed=`expr $needed + $size`
	fi
    done
    echo $needed
}

copy_files ()
{
    files="$*"
    needed=$(get_boot_size $*)
    error=

    get_free_space
    if [ $free -lt $needed ]; then
	return 1
    fi

    for i in $files; do
	if [ -f /boot/$i -o -d /boot/$i ]; then
	    case $i in
		opt.old)
		    if [ ! -f /boot/opt.img ]; then
			cp -a /boot/$i $tmpdir/opt.img || error=true
		    fi
		    ;;
		*) cp -a /boot/$i $tmpdir || error=true ;;
	    esac
	fi
    done

    if [ "$error" ]; then
	for i in $file; do
	    rm -rf $tmpdir/$i
	done
	return 1
    fi
}
save_boot_files ()
{
    ret=0
    boot_mounted=
    if [ ! -f /boot/rc.cfg ]; then
	bdev=`sed -n -e 's#/dev/\([^[:space:]]*\).*[[:space:]]/boot[[:space:]].*\(ro\|defaults\)#\1\2#p' /etc/fstab`
	case $bdev in
	    hd* | sd* | ntfl* | vd* | xvd* )
		mount /boot
		boot_mounted=1
		;;
        esac
    fi
    if [ -f /boot/rc.cfg ]; then
	if grep -q "^BOOT_TYPE='hd'" /boot/rc.cfg; then
	    mkdir $tmpdir
	    if copy_files $boot_file_list ; then
		if ! copy_files $boot_file_list2; then
		    $msg recovery_saving_failed
		    ret=1
		fi
	    else
		# failed to copy standard version
		$msg standard_saving_failed
		ret=1
	    fi
	fi
    fi
    [ -n "$boot_mounted" ] && umount /boot
    return $ret
}

disable_swap ()
{
    if grep -q $1 /proc/swaps; then
	swapoff -a
    fi
}

get_mountpoint_by_device_number()
{
    sed -n "s/[0-9]\+ [0-9]\+ $1 [^ ]\+ \([^ ]\+\).*/\1/p" /proc/self/mountinfo |
	head -n 1
}

get_mountpoints()
{
    for mnt in $(sed -n "s,^/dev/$1 \([^ ]\+\).*,\1,p" /proc/mounts)
    do
	devnum=$(sed -n "s|^[0-9]\+ [0-9]\+ \([0-9]\+:[0-9]\+\) [^ ]\+ $mnt .*|\1|p" \
	    /proc/self/mountinfo)
	mnt2=$(get_mountpoint_by_device_number $devnum)
	if [ "$mnt" = "$mnt2" ]
	then
		echo "/dev/$1:$mnt"
	else
		mproot=$(sed -n "s|^[0-9]\+ [0-9]\+ $devnum \([^ ]\+\) $mnt .*|\1|p" \
		    /proc/self/mountinfo)
		echo "-${mnt2%/}$mproot:$mnt"
	fi
    done
}

get_partitions()
{
    result=
    for volume in $(ls /sys/class/block/); do
	[ -d /sys/block/$1/$volume ] && result="$result $volume"
    done
    echo ${result# }
}

partition_mounted ()
{
    for mntspec in $(get_mountpoints $1)
    do
        mnt=${mntspec#*:}
	vol=${mntspec%%:*}
	if [ "$mnt" = "/boot" ]
	then
	    break
	fi
    done

    for mntspec in $(get_mountpoints $1)
    do
        mnt=${mntspec#*:}
	vol=${mntspec%%:*}
	if [ "$mnt" = "/boot" ]
	then
	    continue
	elif [ "$mnt" = "/var/lib/persistent" ] && echo "$vol" | grep -q "^-/boot\(/.*\)\?"
	then
	    continue
	fi
	return 0
    done
    return 1
}

disk_mounted()
{
    for partition in $(get_partitions $1)
    do
	partition_mounted $partition && return 0
    done
    return 1
}

#----------------------------------------------------------------------------
# set language based on existing language files
#----------------------------------------------------------------------------
for n in de en fr nl hu
do
    msg=/usr/lib/hdinstall/hdinstallmsg_$n.sh
    if [ -f $msg ]; then
	break
    fi
done
[ -f $msg ] || error "Unable to locate message file, aborting." exit

#----------------------------------------------------------------------------
# find device to install to
#----------------------------------------------------------------------------
disks=
for i in `ls /sys/block | sed -n '/\(x\|\)[vsh]d[a-h]/p'` nftla
do
    if grep -q " $i" /proc/partitions; then
	disks="${disks}${sep}${i}"
	sep=', '
	if ! disk_mounted $i; then
	    : ${targetdisk:="$i"}
	else
	    disks="$disks (mounted)"
	fi
    fi
done
echo

#----------------------------------------------------------------------------
# check if hd can be accessed
#----------------------------------------------------------------------------
$msg info

if [ -z "$targetdisk" ]; then
    if [ -z "$disks" ]; then
	targetdisk='<no drive found>'
    else
	targetdisk='<no usable drive found, cannot install on mounted disk>'
    fi
fi
echo    "Drives:       $disks"
echo -n "Installdrive: $targetdisk "
read cancel
case $cancel in
    X|x) exit ;;
    "")  ;;
    *)   targetdisk=$cancel ;;
esac

grep -q " $targetdisk" /proc/partitions || \
    error "Hard disk $targetdisk cannot be found, please choose another disk and check OPT_HDDRV!" exit
if disk_mounted $targetdisk; then
    error "Cannot perform installation onto a disk with mounted partitions, please unmount partitions using"
    type_b=
    for partition in $(get_partitions $targetdisk); do
	for mntspec in $(get_mountpoints $partition); do
	    mnt=${mntspec#*:}
	    error "    umount $mnt # (unmount /dev/$partition)"
	    case $mnt in
		/opt) type_b=1 ;;
	    esac
	done
    done
    if [ -n "$type_b" ]; then
	error "Furthermore, you cannot run hdinstall.sh on a type B installation."
    fi
    error "" exit
fi

check_partition
#----------------------------------------------------------------------------
# select installation type
#----------------------------------------------------------------------------
$msg installation_type

read fli_version
case "$fli_version" in
    X|x)    exit ;;
    A|a|"") fli_version=a ;;
    B|b)    fli_version=b ;;
    *)      error "Please enter A or B!" exit ;;
esac

#----------------------------------------------------------------------------
# description of variables used for partition-calculation:
# heads         number of heads
# cylinders     number of cylinders
# sectors       number of sectors per cylinder
# kb_per_cylinder size of one cylinder in kB
# start         first cylinder of the partition
# end           last cylinder of the partition

calc_cylinders ()
{
    val=`expr '(' $2 '*' 1024 + $kb_per_cylinder - 1 ')' / $kb_per_cylinder`
    eval $1=\$val
}

calcpart ()
{
    # $1 = selected size in MB
    # $2 = maximum size limit in MB

    # recalc maximum-limit to number of cylinders
    partnum=`expr $partnum + 1`
    start=`expr $end + 1`

    if [ "$2" ]; then
	calc_cylinders limit $2
    else
	limit=$available
    fi

    if [ "$1" = "1" ]; then
        # maximum partition size selected
	size=$limit
    else
        calc_cylinders size $1

        if [ $size -gt $limit ]; then
            # limit partition
            size=$limit
        fi
    fi

    if [ $size -gt $available ]; then
	size=$available
    fi
    end=`expr $start + $size - 1`
    available=`expr $available - $size`
    allocated_size=`expr $kb_per_cylinder '*' $size`
    available_size=`expr $kb_per_cylinder '*' $available`
}

#----------------------------------------------------------------------------
# detect geometry and number of kB per cylinder
#----------------------------------------------------------------------------
geometry=$(fdisk -l /dev/$targetdisk | grep head)
cylinders=$(echo "$geometry" | sed -n 's/.*\<\([0-9]\+\) cylinder.*/\1/p')
heads=$(echo "$geometry" | sed -n 's/.*\<\([0-9]\+\) head.*/\1/p')
sectors=$(echo "$geometry" | sed -n 's/.*\<\([0-9]\+\) sector.*/\1/p')

available=$cylinders
kb_per_cylinder=`expr $heads \* $sectors / 2`
partnum=0
end=0

#----------------------------------------------------------------------------
#  dos-partition
#----------------------------------------------------------------------------
size_needed=0`get_boot_size $boot_file_list $boot_file_list2`
size_needed=`expr $size_needed / 1024 + 1`
if [ $size_needed -lt 2 ]
then
    size_needed=2
fi
$msg dos_partition $size_needed
read_num
dos_part=$val

if [ $dos_part -lt $size_needed -a $dos_part -ne 1 ]
then
    error "You need at least $size_needed MiB for your boot partition, but you chose $dos_part MiB, which is too little." exit
fi

calcpart $dos_part 128

# select correct partition type
if [ $allocated_size -lt 8192 ]; then
    type=$part_type_fat12
elif [ $allocated_size -lt 32768 ]; then
    type=$part_type_fat16_max_32
else
    type=$part_type_fat16
fi

add_partition $start $end $type activate

hd_boot=$targetdisk$partnum
hd_boot_size=$allocated_size
echo "hd_boot='$hd_boot'" >/tmp/hdconfig-$$

#----------------------------------------------------------------------------
# ask for opt-partition at setup type B only
#----------------------------------------------------------------------------
if [ "$fli_version" = "b" ]; then
    if [ $available_size -gt 0 ]; then
	$msg opt_partition
	read_num
	opt_part=$val

	case $opt_part in
	    0) ;;
	    *)
	        calcpart $opt_part 512

		add_partition $start $end $part_type_linux
		hd_opt=$targetdisk$partnum
		hd_opt_size=$allocated_size
		echo "hd_opt='$hd_opt'" >>/tmp/hdconfig-$$
		;;
	esac
    else
	error "You chose type B, but there is not enough space for an additional partition on your disk."
	error "Try a smaller size for the boot partition or use type A." exit
    fi
fi

#----------------------------------------------------------------------------
# ask for optional swap-partition only if less than 32MB RAM and no flashdisk
#----------------------------------------------------------------------------
if echo $targetdisk | grep -q ^hd ; then
    flashmedia="`hdparm -i /dev/$targetdisk | grep -i "nonmagnetic"`"
fi
totalram=`sed -n 's/^MemTotal://p' /proc/meminfo | sed 's/ kB//;'`

if [ $available_size -gt 0 -a $totalram -lt 30000 -a "$flashmedia" = "" ]; then
    $msg swap_partition
    read_num
    swap_part=$val

    case "$swap_part" in
	0) ;;
        *)
            calcpart $swap_part 256
	    add_partition $start $end $part_type_linux_swap
	    hd_swap=$targetdisk$partnum
	    hd_swap_size=$allocated_size
	    echo "hd_swap='$hd_swap'" >>/tmp/hdconfig-$$
	    ;;
    esac
fi

#----------------------------------------------------------------------------
# ask for optional data-partition
#----------------------------------------------------------------------------
if [ $available_size -gt 0 ]; then
    $msg data_partition
    read_num
    data_part=$val

    case "$data_part" in
	0) ;;
        *)
            calcpart $data_part
	    add_partition $start $end $part_type_linux
	    hd_data=$targetdisk$partnum
	    hd_data_size=$allocated_size
	    echo "hd_data='$hd_data'" >>/tmp/hdconfig-$$
	    ;;
    esac
fi

#----------------------------------------------------------------------------
# show warning before deleting any data on disk
#----------------------------------------------------------------------------
show_allocation ()
{
    if [ "$2" ]; then
	echo "    $1: $2 - `expr $3 / 1024`MB"
    else
	echo "    $1: none"
    fi
}
if [ "$ptable_modified" = yes ]
then
  $msg final_warning_1
else
  $msg final_warning
fi

cat <<EOF

#
# Partitions
#

EOF
show_allocation "Boot " "$hd_boot" "$hd_boot_size"
show_allocation "Opt  " "$hd_opt"  "$hd_opt_size"
show_allocation "Swap " "$hd_swap" "$hd_swap_size"
show_allocation "Data " "$hd_data" "$hd_data_size"
echo
read all_ok
case "$all_ok" in
    JA | YES | OUI) ;;
    *)        exit ;;
esac

#----------------------------------------------------------------------------
# repartition the harddisk
#----------------------------------------------------------------------------
if ! save_boot_files; then
    read all_ok
    case "$all_ok" in
	JA | YES | OUI) ;;
	*)        exit ;;
    esac
fi

mntspecs=
for partition in $(get_partitions $targetdisk)
do
    mntspecs="$mntspecs $(get_mountpoints $partition)"
done

for mntspec in $mntspecs
do
    mnt=${mntspec#*:}
    if ! umount $mnt
    then
	error "Unmounting $mnt failed! The following programs are holding open files:"
	lsof | sed -n "s|^\([0-9]\+\)[[:space:]]\([^ ]\+\)[[:space:]]$mnt.*|\2 (PID \1)|p" | sort -u
	error "" exit
    fi
done

disable_swap /dev/"$targetdisk"

# write clean mbr to get a clean partition table and to remove any old bootloader
dd if=/dev/zero of=/dev/"$targetdisk" bs=512 count=2 >/dev/null 2>/dev/null
dd if=/usr/share/syslinux/mbr.bin of=/dev/"$targetdisk" >/dev/null 2>/dev/null

# create new partitions
echo "w" >>/tmp/newpart-$$
if ! fdisk -C $cylinders -H $heads -S $sectors /dev/"$targetdisk" </tmp/newpart-$$ >>/tmp/fdisk.$$ 2>&1; then
    error "Creating new partition table failed!"
    error "fdisk output:"
    cat /tmp/fdisk.$$
    error "" exit
fi

#----------------------------------------------------------------------------
# format all new created partitions
#----------------------------------------------------------------------------
. /tmp/hdconfig-$$
clrhome
echo formatting partitions...

for i in $hd_boot $hd_opt $hd_swap $hd_data; do
    if [ ! -b /dev/$i ]; then
	echo -n "Waiting for /dev/$i to appear "
	while [ ! -b /dev/$i ]; do
	    echo -n "."
	    sleep 1
	done
	echo " done"
    fi
done

mkfs.fat /dev/"$hd_boot" || error "Formatting boot partition $hd_boot failed!" exit
hd_boot_uuid=`blkid | sed -n "s#/dev/${hd_boot}:.*UUID=\"\([-a-fA-F0-9]*\)\".*#\1#p"`
if [ -n "$hd_boot_uuid" ]
then
    echo "hd_boot_uuid='$hd_boot_uuid'" >>/tmp/hdconfig-$$
fi

if [ "$hd_opt" != "" ]; then
    mke2fs -j /dev/"$hd_opt" || error "Formatting OPT partition $hd_opt failed!" exit
    hd_opt_uuid=`blkid | sed -n "s#/dev/${hd_opt}:.*UUID=\"\([-a-fA-F0-9]*\)\".*#\1#p"`
    if [ -n "$hd_opt_uuid" ]
    then
	echo "hd_opt_uuid='$hd_opt_uuid'" >>/tmp/hdconfig-$$
    fi
fi

if [ "$hd_swap" != "" ]
then
    mkswap /dev/"$hd_swap" || error "Preparing swap partition $hd_swap failed!" exit
    # create uniqe UUID with dd (that's the wood-hammer-method, but works)
    dd if=/dev/urandom of=/dev/"$hd_swap" seek=1036 bs=1 count=16

    hd_swap_uuid=`blkid | sed -n "s#/dev/${hd_swap}:.*UUID=\"\([-a-fA-F0-9]*\)\".*#\1#p"`
    if [ -n "$hd_swap_uuid" ]
    then
	echo "hd_swap_uuid='$hd_swap_uuid'" >>/tmp/hdconfig-$$
    fi
fi

if [ "$hd_data" != "" ]
then
    mke2fs -j /dev/"$hd_data" || error "Formatting data partition $hd_data failed!" exit
    hd_data_uuid=`blkid | sed -n "s#/dev/${hd_data}:.*UUID=\"\([-a-fA-F0-9]*\)\".*#\1#p"`
    if [ -n "$hd_data_uuid" ]
    then
	echo "hd_data_uuid='$hd_data_uuid'" >>/tmp/hdconfig-$$
    fi
fi

#----------------------------------------------------------------------------
# execute syslinux
#----------------------------------------------------------------------------
echo "running syslinux ..."
if ! syslinux -si /dev/"$hd_boot"; then
    error "Running syslinux failed! Disk will not be bootable."
    error "Try making the boot partition a little bigger or smaller." exit
fi

# NOTE: If we install on another medium (e.g. CD->HD, HD1->HD2 etc.) we have
# /boot already mounted (if MOUNT_BOOT != 'no'). However, this is not much of
# a problem, because busybox mount happily "overmounts" /boot, i.e. after the
# mount, we have _two_ devices being mounted at /boot, however (obviously) only
# the last mount is effective. If we unmounted /boot later, the previous mount
# would be reactivated. Summarising so far, we are not obliged to unmount /boot
# before mounting another device on it.
mount -w /dev/"$hd_boot" /boot -t vfat || error "Mounting boot partition $hd_boot failed!" exit

cp -a /tmp/hdconfig-$$ /boot/hd.cfg || error "Copying hd.cfg to /boot failed!" exit

if [ -f $tmpdir/rc.cfg ]; then
    # We always copy the boot files to the target, even if they might not be
    # compatible with the target drive (e.g. CD->HD setup). For this reason,
    # we will display an appropriate message to the user demanding a remote
    # update before the next reboot.
    if cp -a $tmpdir/* /boot; then
	sync

	# determine old boot volume
	bootvol=
	for mntspec in $mntspecs
	do
	    device=${mntspec%%:*}
	    mnt=${mntspec#*:}
	    if [ "$mnt" = "/boot" ]
	    then
		bootvol=$device
		break
	    fi
	done

	if [ "/dev/$hd_boot" = "$bootvol" ]
	then
	    # old and new boot volumes are the same => repartitioning
	    umount /boot
	    for mntspec in $mntspecs
	    do
		device=${mntspec%%:*}
		mnt=${mntspec#*:}
		case $device in
		    -*) mount --bind ${device#-} $mnt ;;
		    *)  mount $device $mnt ;;
		esac
	    done
	    $msg finish_repartitioning
	else
	    # old and new boot volumes differ => HD->HD installation
	    # (e.g. booting from CF card or USB stick and installing to HD)
	    # => leave /boot "as is" and don't remount anything, such that a
	    # remote update will update the correct boot files on the target
	    # device
	    $msg finish
	fi
    else
	error "Copying boot files to /boot failed, press <RETURN> to continue"
	read foo
	$msg finish
    fi
else
    $msg finish
fi
cleanup
