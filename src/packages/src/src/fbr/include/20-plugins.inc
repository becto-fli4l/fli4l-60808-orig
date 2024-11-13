#!/bin/bash

if [ -f "$fbr_root/include/plugins/$1.inc" -o -f "$fbr_workdir/include/plugins/$1.inc" ]
then
	plugin="$1"
	shift
	message "Executing plugin $plugin" standout
	if [ -f "$fbr_workdir/include/plugins/$plugin.inc" ]
	then
		. "$fbr_workdir/include/plugins/$plugin.inc" action "$@"
		exit $?
	else
		. "$fbr_root/include/plugins/$plugin.inc" action "$@"
		exit $?
	fi
fi
