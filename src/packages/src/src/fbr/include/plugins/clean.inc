#!/bin/bash

case "$1" in
help)
	message "  $progname clean" yellow
	message "    removes the FBR's output files"
	;;

action)
	message "About to delete the FBR's output files in $br_wcout" standout
	warning "Press RETURN to proceed or Ctrl+C to cancel!"
	read
	message "Deleting the FBR's output files in $br_wcout" standout
	[ -d $br_wcout ] && rm -rf $br_wcout || (exit 0)
	;;
esac
