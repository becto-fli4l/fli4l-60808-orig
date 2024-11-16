#!/bin/bash

packages="$@"
if [ -n "$packages" ]
then
	files=
	for pkg in $packages
	do
		files+=" $(dirname $0)/packages/$pkg/files.txt"
	done
else
	files=$(ls -1 $(dirname $0)/packages/*/files.txt)
fi

for filestxt in $files
do
	[ -f "$filestxt" ] || continue
	echo -n "."
	LC_ALL="C" sort -k 2 -u -o "$filestxt" "$filestxt"
done
echo
