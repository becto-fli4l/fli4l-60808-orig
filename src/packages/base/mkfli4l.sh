#! /bin/sh
# -------------------------------------------------------------------------
# creates a fli4l-Bootmedia or Image                           __FLI4LVER__
#
# Creation:       2004-10-25 LanSpezi
# Last Update:    $Id$
# -------------------------------------------------------------------------

remote_remount()
{
    if [ "$remoteremount" = true ]; then
        echo " -> remount target fs $1"
        ssh $ssh_port_opt ${remoteusername}@${remotehostname} "mount -o remount,$1 ${remotepathname}"
        remount_str="sync ; mount -o remount,ro  ${remotepathname}"
    else
        remount_str=true
    fi
}
# include compatibility checks for Darwin (Mac OS X)
. ./unix/scripts/compatibility.sh

# include tool lookup function
. ./unix/scripts/lookup_tool.sh

# include cleanup-functions
. ./unix/scripts/_lib_cleanup.sh

# include function to check command-line
. ./unix/scripts/parse_cmd.sh

# remove any obsolete log file
rm -f /tmp/mkfli4l_error.log

# check parameters from commandline and current env
parse_cmdline "$@"

if [ "$bool_cleanup" = "true" ]; then
    cleanup_fli4lfiles
    cleanup_mkfli4l
    exit 1
fi

# files to install
targets=
. unix/scripts/mkopt.sh

if [ "$remoteupdate" != "true" -a "$bool_filesonly" != "true" ]; then
    case "$boot_type" in
        pxeboot)    . unix/scripts/mkpxeboot.sh  ;;
        cd)         . unix/scripts/mkiso.sh      ;;
        ls120)      abort "- build of a bootable ls120-disk is not available" ;;
        netboot)    . unix/scripts/mknetboot.sh  ;;
        hd|attached|integrated)
            if [ "$hdinstall_path" ]; then
                . unix/scripts/mkhdinstall.sh
            fi
            ;;
        *)
            # otherboot
            if [ -f unix/scripts/mk${boot_type}.sh ]; then
                . unix/scripts/mk${boot_type}.sh
            else
                log_error "- can not execute script for BOOT_TYPE=$boot_type"
                log_error "- file not found: unix/scripts/mk$boot_type.sh"
            fi
            ;;
    esac
fi

if [ "$remoteupdate" = "true" ]; then
    echo "--- copying files to the router ----------------------------------------------"

    scp_port_opt=
    ssh_port_opt=
    if [ "$remoteport" ]; then
        scp_port_opt="-P $remoteport"
        ssh_port_opt="-p $remoteport"
    fi
    if [ "$sshkeyfile" ]; then
        scp_port_opt="$scp_port_opt -i $sshkeyfile"
        ssh_port_opt="$ssh_port_opt -i $sshkeyfile"
    fi

    remote_remount rw

    if scp $scp_port_opt -r "$dir_image"/* ${remoteusername}@${remotehostname}:${remotepathname}; then
        if [ "$bool_yes" = "true" -a "$bool_batch" = "true" ]; then
            ssh $ssh_port_opt ${remoteusername}@${remotehostname} "$remount_str ; if [ -f /usr/local/bin/check-bootfiles.sh ];then /usr/local/bin/check-bootfiles.sh --log-version --reboot; else /sbin/reboot; fi"
        else
            echo ""
            echo "--- reboot the router? - ( after check of updated files ) --------------------"
            echo " type 'yes' to reboot"
            read dummy
            # convert input to lower case
            dummy=`echo $dummy | sed "s/[Yy][Ee][Ss]/yes/"`
            if [ "$dummy" = "yes" ]; then
                ssh $ssh_port_opt ${remoteusername}@${remotehostname} "$remount_str ; if [ -f /usr/local/bin/check-bootfiles.sh ];then /usr/local/bin/check-bootfiles.sh --log-version --reboot; else /sbin/reboot; fi"
            else
                ssh $ssh_port_opt ${remoteusername}@${remotehostname} "$remount_str ; if [ -f /usr/local/bin/check-bootfiles.sh ];then /usr/local/bin/check-bootfiles.sh --log-version ; fi"
                remote_remount ro
                echo "please reboot your router to activate new bootfiles"
            fi
        fi
    else
        remote_remount ro
        echo "failed to update the router"
        exit 1
    fi
fi
exit 0
