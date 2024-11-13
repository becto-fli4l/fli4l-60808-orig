#! /bin/sh

#
# here documents have to be indented by tabs, if you expect the 
# indentation to be removed by cat <<-EOF
#

if [ -n "$2" ]
then
  param2=$2
else
  param2=''
fi

clrhome
case $1 in

    info)
        cat <<-EOF
		fli4l - Installation on a hard disk / Compact-Flash
		
		Choose a hard disk:
		
		Please hit the enter-Button to confirm the installation to the suggested 
		hard disk. You may enter another valid hard disk also.
		
		To install to other hard disks, enter one of the following strings:
		  primary IDE port, master        hda
		  primary IDE port, slave         hdb
		  secondary IDE port, master      hdc
		  secondary IDE port, slave       hdd
		  first (S)ATA/SCSI-Disk          sda
		
		Please enter a valid hard disk string or enter X to exit the setup program: 
		
EOF
        ;;

    inv_partition_table)
        cat <<-EOF
		fli4l - Installation on a hard disk / Compact-Flash
		 
		The partition table is in an inconsistent state.
		To ensure a proper installation we have to write a valid partition table to
		your disk before continuing with the installation.
		Writing a new partition table will destroy all data on your disk.
		
		*****************************************************************
		* This is your last chance before the forthcoming loss of data! *
		*    All data on your hard disk will be DELETED permanently!    *
		*****************************************************************
		
		Please enter YES in capital letters to continue: 
EOF
        ;;

    installation_type)
        cat <<-EOF
		fli4l - Installation on a hard disk / Compact-Flash
		
		Select the installation type
		 
		type A: Run completely from the ramdisk
		        All required files are stored on one DOS-partition.
		        At boottime, everything is extracted to a ramdisk und the router
		        and all programs run from out of the ramdisk. The hard disk is only
		        used as a big floppy drive.
			
		type B: Use a ext3-partition in addition to a small ramdisk
		        Only some essential boot-files are stored on the DOS-partition.
		        Files included in the opt.img are decompressed automatically 
		        to the ext3-partition and are run from this place.
		  
		
		Please choose on: A or B (X to abort the installation)
EOF
        ;;

    dos_partition)
        if [ -z "$param2" ]
        then
          param2=2
        fi
        cat <<-EOF
		fli4l - Installation on a hard disk / Compact-Flash
		  
		Create DOS-Partition:
		   
		How large should the DOS-partition be?
		
		The minimal size is $param2 MB, recommended is between 4 and 8 MB.
		The maximal size is 128 MB, larger values are reduced to 128 MB automatically!
		Enter 1 to use the remaining space of a Compact-Flash-module with a capacity
		lower than 128 MB for the DOS-partition. 
		
		Please enter the desired size in MB, enter 1 to use the maximum partition
		size or enter x to exit the setup program:
EOF
        ;;

    opt_partition)
        cat <<-EOF
		fli4l - Installation on a hard disk / Compact-Flash
		
		Create opt-partition:
		
		How large shall the partition for the opt.img be?
		
		The minimum size is 2 MB, recommended is between 4 and 16 MB.
		The maximum value is 512 MB. 
		This partition is used instead of the ramdisk. 
		
		Please enter the desired size in MB, enter 1 to use the maximum partition
		size or enter x to exit the setup program: 
EOF
        ;;

    swap_partition)
        cat <<-EOF
		fli4l - Installation on a hard disk / Compact-Flash
		 
		Create swap-partition:
		 
		Do you want to create a swap-partition?
		
		You only need a swap-partition, if you would like to use many programs or a
		big partition and only have little memory. The swap-partition should
		be at least as big as your 'real' memory (RAM). Maximum value is 256 MB, bigger
		values are automatically reduced to 256 MB.
		
		You should only create a swap-partition on a hard disk. If you install to a
		Compact-Flash, you shouldn't create a swap-partition!
		
		Please enter the desired size in MB, or enter 1 to use the maximum partition
		size or press enter, if you don't want to create a swap-partition:
EOF
        ;;

    data_partition)
        cat <<-EOF
		fli4l - Installation on a hard disk / Compact-Flash
		 
		Create a data-partition:
		 
		Do you want to create an extra data-partition?
		
		This is only needed if you need some storage on your Router to use opt_vbox
		or opt_samba_lpd for example.
		An ext3-partition will be created for this.
		The maximal size is 2 TB, larger values are reduced automatically!
		
		Please enter the desired size in MB, or enter 1 to use the maximum partition
		size or press enter, if you don't want to create a data-partition:
EOF
        ;;

    final_warning)
        cat <<-EOF
		fli4l - Installation on a hard disk / Compact-Flash
		 
		Last question before repartitioning:
		
		*****************************************************************
		* This is your last chance before the forthcoming loss of data! *
		*    All data on your hard disk will be DELETED permanently!    *
		*****************************************************************
		
		Please enter YES in capital letters to create the 
		following partitions:
EOF
        ;;

    final_warning_1)
    	cat <<-EOF
		fli4l - Installation on a hard disk / Compact-Flash
		
		Security check before Repartitioning
		
		Please enter YES in capital letters to create the 
		following partitions:
EOF
	;;

    recovery_saving_failed)
        cat <<-EOF
		fli4l - Installation on a hard disk / Compact-Flash
		
		Failed to create a temporary copy of the recovery
		version. You have to create a new one after
		repartitioning your disk.
		
		Please enter YES in capital letters to continue:

EOF
        ;;

    standard_saving_failed)
        cat <<-EOF
		fli4l - Installation on a hard disk / Compact-Flash
		
		Failed to create a temporary copy of the standard boot
		files. You have to transfer a new version to the
		router using the remote update facility after
		repartitioning your disk.
		
		Please enter YES in capital letters to continue:

EOF
        ;;

    finish)
        cat <<-EOF
		fli4l - Installation on a hard disk / Compact-Flash
		 
		What shall I do next?
		
		The harddisk was prepared and is mounted at /boot now. Please adapt
		your configuration to your needs and transfer the created files with
		imonc, fli4l-Build or scp to the harddisk to the directory /boot.
		
		The following files are needed to boot from harddisk:
		syslinux.cfg, kernel, rootfs.img, rc.cfg und opt.img
		
		ATTENTION: You have to transfer the files mentioned above to the
		router now (before rebooting it), otherwise it will not be able to
		boot from hd!
		
		After the remote update remove the setup-floppy from your router, shut
		it down and reboot it (using reboot/halt/poweroff). Do not just reset
		your router, otherwise your last changes may be lost.
		
EOF
        ;;

    finish_repartitioning)
        cat <<-EOF
		fli4l - Installation on a hard disk / Compact-Flash
		 
		What shall I do next?
		
		The harddisk was re-partitioned and formatted, you may reboot your
		router now.
		
		If you would like to update the router, you may do this now.
		
		Reboot the router afterwards using
		reboot/halt/poweroff. Do not just reset your router,
		otherwise your last changes may be lost.

EOF
        ;;
    *)
        echo "Unknown text message requested, please inform author."
	exit 1
	;;
esac
exit 0
