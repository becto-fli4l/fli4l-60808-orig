#!/bin/bash

case "$1" in
help)
	message "  $progname help" yellow
	message "    displays this help"
	;;

action)
	message "=============================================================" yellow
	message "|   F B R - M A K E   U S A G E   I N S T R U C T I O N S   |" yellow
	message "=============================================================" yellow
	message
	message "  $progname world" yellow
	message "    builds the complete FBR"
	message
	message "  $progname toolchain" yellow
	message "    builds the FBR toolchain only"
	message
	message "  $progname <package>+" yellow
	message "    builds the toolchain, the given packages and their dependencies"
	message
	message "  $progname <package>-clean" yellow
	message "    removes the package's files from the staging and target directories and"
	message "    removes its build and sandbox directories, allowing a rebuild"
	message
	message "  $progname show-versions" yellow
	message "    show the versions of all configured packages"
	message
	message "  $progname buildroot-menuconfig" yellow
	message "    reconfigures the uClibc buildroot"
	message
	message "  $progname <pkg>-menuconfig" yellow
	message "    reconfigures the package <pkg> (only supported by few packages,"
	message "    e.g. uclibc, busybox, linux-<version>)"
	message

	for f in $(ls -1 {$fbr_root,$fbr_workdir}/include/plugins/*.inc 2>/dev/null)
	do
		. $f help
		message ""
	done

	exit 10
	;;
esac
