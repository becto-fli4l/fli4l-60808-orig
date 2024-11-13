#!/bin/bash

command=$1
shift

case "$command" in
help)
	message "  $progname update-src-package <path-to-fli4l-SVN-working-copy>" yellow
	message "    correct entries in the file list pointing to new or obsolete files"
	;;

action)
	message "Updating file list of the src package in SVN working copy" standout
	svnroot="$1"

	[ -n "$svnroot" ] || \
		error "Directory of fli4l SVN working copy is missing" 100

	[ -d "$svnroot" ] || \
		error "$svnroot does not point to a fli4l SVN working copy" 101

	pkgroot="$svnroot/src/packages/src"
	[ -d "$pkgroot" ] || error "$svnroot/src/packages does not exist" 102

	checkfiles="$svnroot/src/check-files.pl"
	[ -x "$checkfiles" ] || error "$svnroot/src/check-files.pl does not exist or is not executable" 104

	message "Deleting obsolete entries..."
	$checkfiles files 2>/dev/null |
		sed -n "s/^! src //p" |
		sed -e 's,/,\\/,g' -e 's,.*,/^. &$/d,' > /tmp/sed-script.$$
	sed -i -f /tmp/sed-script.$$ $pkgroot/files.txt
	rm -f /tmp/sed-script.$$

	message "Adding missing entries..."
	$checkfiles files 2>/dev/null |
		sed -n "s,^? src \(src/fbr/buildroot/\),\1,p" |
		while read name
		do
			[ -x "packages/src/$name" ] && type='B' || type='b'
			echo "$type $name"
		done > /tmp/sed-script.$$
	cat /tmp/sed-script.$$ >> $pkgroot/files.txt
	rm -f /tmp/sed-script.$$

	message "Sorting file list..."
	$svnroot/src/sort-files_txt.sh src
	;;
esac
