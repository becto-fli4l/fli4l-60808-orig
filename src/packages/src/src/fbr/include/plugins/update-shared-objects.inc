#!/bin/bash

command=$1
shift

case "$command" in
help)
	message "  $progname update-shared-objects [options...] <path-to-fli4l-SVN-working-copy>" yellow
	message "    correct entries linking to obsolete shared objects in the packages' file lists"
	;;

action)
	message "Updating packages' file lists in SVN working copy" standout
	svnroot="$1"

	[ -n "$svnroot" ] || \
		error "Directory of fli4l SVN working copy is missing" 100

	[ -d "$svnroot" ] || \
		error "$svnroot does not point to a fli4l SVN working copy" 101

	pkgroot="$svnroot/src/packages"
	[ -d "$pkgroot" ] || error "$svnroot/src/packages does not exist" 102

	binroot="$svnroot/bin/$fbr_arch"
	[ -d "$binroot" ] || \
		error "$binroot does not point to a directory containing binaries for the $fbr_arch architecture" 103

	checkfiles="$svnroot/src/check-files.pl"
	[ -x "$checkfiles" ] || error "$svnroot/src/check-files.pl does not exist or is not executable" 104

	$checkfiles files 2>/dev/null |
		sed -n 's/^! \([^ ]\+\) opt\/\(.*\.so\.[^/]\+\)$/\1 \2/p' |
		while read package oldfile
		do
			oldfileunversioned=$(echo "$oldfile" | sed 's/^\(.*\.so\)\.[^/]\+$/\1/')
			dir=$(dirname "$oldfileunversioned")
			oldnameunversioned=$(basename "$oldfileunversioned")
			newfile=$(find "$binroot/$dir/" -mindepth 1 -maxdepth 1 -type f -name "$oldnameunversioned*" | head -n 1)
			if [ -n "$newfile" ]
			then
				newfile=$dir/$(basename "$newfile")
				message "  $package: $oldfile --> $(basename "$newfile")"
				sed -i "s,^\(. opt/\)$oldfile$,\1$newfile," "$pkgroot/$package/files.txt"
			else
				message "  $package: $oldfile --> ???"
			fi
		done
	;;
esac
