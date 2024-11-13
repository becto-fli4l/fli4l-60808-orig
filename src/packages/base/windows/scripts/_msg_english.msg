
; -------------------------------------------------------------------------
; language constants (english)                                 __FLI4LVER__
;
; Creation: lanspezi 2005-04-14
; $Id$
; -------------------------------------------------------------------------

[Global]
MSG_LINE===================================================================
MSG_BTN_close=Close
MSG_msgbox_error=Error
MSG_msgbox_warn=Warning
MSG_msgbox_info=Info
MSG_errorfilelog=Unable to open log-file!
MSG_errorfilelogerror=Unable to open error-log-file!
MSG_finished=finished.
MSG_cancel=cancelled.
MSG_00_scpstart=Transferring files with SCP to the router ...
MSG_00_scpend=Finished transferring files.
MSG_00_scpend2=Now reboot your fli4l router.
MSG_00_pass=Password for SCP-Transfer
MSG_00_usebasepass=Use password noted in 'base.txt'?
MSG_00_scpenterpass=Enter your password for SCP-File transfer to host:
MSG_00_scpcancel=SCP-File transfer cancelled
MSG_00_scperror1=An error occurred during SCP-File transfer
MSG_00_scperror2=Please check all SSH-Parameters and the SSHD on the remote host
MSG_00_changesshkey=Select SSH-Keyfile
MSG_00_plinkaskreboot=Do you want to reboot the router?
MSG_00_plinksentreboot=The reboot command was sent to the router.
MSG_00_askreboot=fli4l - Reboot router

[mkfli4l]
MSG_01_build_abort=The build-process was cancelled due to user request
MSG_01_build_error=The build-process was aborted by an error
MSG_01_no_boot_type=Non supported value of BOOT_TYPE

[parse_cmd]
MSG_02_start=__FLI4LVER__ - start of build process
MSG_02_noconfdir=In directory with fli4l-config no base.txt exists
MSG_02_useconfig=Using configuration from directory '
MSG_02_undefboottype=Undefined variable BOOT_TYPE, please use config-files in correct format
MSG_02_migrateconfig=Migrating mkfli4l.conf to mkfli4l.txt...

[parse_mkfli4l_conf]
MSG_03_readcheck_buildenv=Reading and checking the build-environment '
MSG_03_nologfile=Unable to open log file!

[mkopt]
MSG_04_start=Creating fli4l-build-files ...
MSG_04_confdir=Use Configuration in directory '
MSG_04_builddir=Create build-files in directory '
MSG_04_readcheckconf=Reading and checking the configuration, creating rootfs- and opt-archive ...
MSG_04_generating_md5sums=Generating md5 checksums ...
MSG_04_finished=Creating of fli4l-build-files finished.
MSG_04_nologfile=Unable to open log file!
MSG_04_msgboxerror=Error in your fli4l-config-files!

[mkfloppy]
MSG_05_start=Creating the floppy/floppies
MSG_05_insertfloppy=Please insert a formatted floppy...
MSG_05_bootdisk=(this will be the BOOT_DISK)
MSG_05_startcopy=Click OK or press ENTER to start copying files to floppy
MSG_05_copysyslinux=Copying syslinux.cfg ...
MSG_05_writesystem=Writing system files ...
MSG_05_copykernel=Copying kernel ...
MSG_05_copyrootfstgz=Copying rootfs.img ...
MSG_05_copyrootfsbz2=Copying rootfs.bz2 ...
MSG_05_copyrccfg=Copying rc.cfg ...
MSG_05_copyopttarbz2=Copying opt.img ...
MSG_05_bootdisk2= (BOOT_DISK)
MSG_05_optdisk= (OPT_DISK)
MSG_05_secfloppy1=Remove the first floppy (BOOT_DISK)
MSG_05_secfloppy2=Insert a formatted floppy and press ENTER or click OK
MSG_05_secfloppy3=(this will be the OPT_DISK)
MSG_05_floppynowrite1=While writing the file '
MSG_05_floppynowrite2=an error occurred!
MSG_05_floppynospace=There is not enough space on the floppy for the build files.
MSG_05_bootflnospace=There is not enough space on the BOOT-floppy for the build files.
MSG_05_cancel=Writing of floppy/floppies cancelled by user.
MSG_05_error=Error on writing to floppy!
MSG_05_finish1=Finished creation of your fli4l-floppy
MSG_05_finish2=- now start your fli4l router with the new floppy/floppies
MSG_05_filesonfloppy=Floppy contains files. Remove?
MSG_05_usefloppy=Use this floppy anyhow?
MSG_05_nofloppy=Can't find a floppy!
MSG_05_nofloppyW9x=Can't find a floppy or floppy is not formatted!
MSG_05_calculatespace=Calculate required space ...
MSG_05_requiredspace1=Required space:
MSG_05_requiredspace2=bytes.
MSG_05_requiredspace3=Available space:
MSG_05_floppyseemsboot=Warning: this floppy seems to be the BOOT_DISK!
MSG_05_exchangefloppy=Exchange the floppy and click OK to continue
MSG_05_floppynotformatted1=Floppy is not formatted!
MSG_05_floppynotformatted2=Format?
MSG_05_formatfloppy=Format floppy ...

[mkiso]
MSG_06_createisoimage=Creating the iso-image ...
MSG_06_buildisoimage=Building the iso-image '
MSG_06_finish1=Finished creation of your fli4l iso-image
MSG_06_finish2=Now you can burn it and start your fli4l-router with the CD

[mknetboot]
MSG_07_mld1=At the moment creation of a fli4l-netboot-image is not possible
MSG_07_mld2=with any version of Microsoft's Windows!

[dlg_mkfli4l_conf]
MSG_08_btnabort=Abort build process
MSG_08_btnbuild=Start build process
MSG_08_nobasetxt='base.txt' does not exist in the new directory
MSG_08_changedirconfig=Directory with your fli4l config-files
MSG_08_changedconfigdir1=Changed Config directory
MSG_08_changedconfigdir2=Use new Config-dir as default on next run?
MSG_08_changedirbuild=Directory for the fli4l-files created by build-process
MSG_08_dirconfiginfo1=Changing of 'Config-Dir' will set all options to variables found in the new 'mkfli4l.txt'
MSG_08_dirconfiginfo2=or to options given by command line
MSG_08_savemkfli4lconf=Save current config to mkfli4l.txt
MSG_08_savemkfli4ldone=Your config was saved to mkfli4l.txt
MSG_08_savemkfli4lerror=Error on writing mkfli4l.txt
MSG_08TT_btnconfigdir=Change to another config-dir
MSG_08TT_btnbuilddir=Change to another build-dir
MSG_08TT_verbose=Activate for more verbose output
MSG_08TT_filesonly=Activate to build files only
MSG_08TT_drive=Select drive for your fli4l-floppy
MSG_08TT_floppy_normal=Select normal floppy mode
MSG_08TT_floppy_safe=Select safe floppy mode (slow)
MSG_08TT_remoteupdate=Activate ssh remote update
MSG_08TT_remotehostname=Host/ip-address to transfer files to
MSG_08TT_remoteusername=Username for the ssh transfer
MSG_08TT_remotepathname=Pathname for the ssh transfer
MSG_08TT_remoteport=Port for the ssh transfer
MSG_08TT_btn_sshfile=Change to an other sshfile
MSG_08TT_btn_sshfileclear=Clear sshfile and use user/password instead

[mkhdinstall]
MSG_09_start=HD-Install on selected Disk
MSG_09_copysyslinux=Copying syslinux.cfg ...
MSG_09_copykernel=Copying kernel ...
MSG_09_copyrootfstgz=Copying rootfs.img ...
MSG_09_copyrootfsbz2=Copying rootfs.bz2 ...
MSG_09_copyrccfg=Copying rc.cfg ...
MSG_09_copyopttarbz2=Copying opt.img ...
MSG_09_writesystem=Writing system files ...
MSG_09_finish1=Finished creation of your hd-pre-install
MSG_09_finish2=- now you can insert the disk to the router and start it
MSG_09_cancel=Writing to disk (HD-pre-install) cancelled by user.
MSG_09_error=Error writing to disk (HD-pre-install)!
MSG_09_admin1=You don't have Admin-Rights - they are needed to make a bootable disk
MSG_09_admin2=Do you want to run 'syslinux' as Administrator?
MSG_09_ask_pass=Password for Administrator
MSG_09_pass=Password:
MSG_09_wrongfs1=Drive
MSG_09_wrongfs2=doesn't have a FAT-Filesystem!
MSG_09_errorcopy=Error on copying file

[mkls120]
MSG_10_start=Install on selected LS120-Disk
MSG_10_copysyslinux=Copying syslinux.cfg ...
MSG_10_copykernel=Copying kernel ...
MSG_10_copyrootfstgz=Copying rootfs.img ...
MSG_10_copyrootfsbz2=Copying rootfs.bz2 ...
MSG_10_copyrccfg=Copying rc.cfg ...
MSG_10_copyopttarbz2=Copying opt.img ...
MSG_10_writesystem=Writing system files ...
MSG_10_finish1=Finished creation of your fli4l-LS120-drive
MSG_10_finish2=- now you can insert the disk to the router and start it
MSG_10_cancel=Writing to LS120-disk cancelled by user.
MSG_10_error=Error writing to disk (LS120)!
MSG_10_wrongfs1=Drive
MSG_10_wrongfs2=doesn't have a FAT-Filesystem!
MSG_10_errorcopy=Error on copying file
