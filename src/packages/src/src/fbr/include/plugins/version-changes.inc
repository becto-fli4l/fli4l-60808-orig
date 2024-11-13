#!/bin/bash

quote()
{
	sed 's/\([\\"]\)/\\\1/g'
}

convert_refs()
{
	sed 's/\$(@[^ )]\+)//g;s/\$(\([^ )]\+\)\(:[^ ]\+\)\?)/${\1}/g'
}

filter_out_functions()
{
	sed 's/^"\$(call qstrip,\(.*\))" \([^ ]\+\)$/"\1" \2/g;/\$([-a-z]\+ /d'
}

read_makefile()
{
	sed -n 's,^[[:space:]]*\([A-Z][A-Z0-9_]*_VERSION[A-Z0-9_]*\)[[:space:]]*:\?=[[:space:]]\(.*[^\\]\)$,"\2" \1,p' |
		filter_out_functions |
		convert_refs |
		uniq -f 1 |
		sed 's/^"\(.*\)" \([^ ]\+\)$/\1 \2/' |
		quote |
		sed 's/^\(.*\) \([^ ]\+\)$/\2="\1"/'
}

case "$1" in
help)
	message "  $progname version-changes" yellow
	message "    prints all package version changes between the FBR source and its"
	message "    working copy"
	;;

action)
	message "Package version changes" standout

	for mk in $(find $br_dir/package/ $br_dir/linux $br_dir/boot $br_dir/fs -mindepth 2 -name '*.mk')
	do
		pkg=$(basename $mk .mk)
		pkgupper=$(echo "$pkg" | tr 'a-z' 'A-Z' | sed 's/[^a-zA-Z0-9]/_/g')

		(
		eval "$(svn cat $br_config | convert_refs)"
		eval "$(svn cat $mk | read_makefile)"
		# called twice to solve forward references
		eval "$(svn cat $mk | read_makefile)"
		eval pkg_old_version=\$${pkgupper}_VERSION

		eval "$(cat $br_config | convert_refs)"
		eval "$(cat $mk | read_makefile)"
		# called twice to solve forward references
		eval "$(cat $mk | read_makefile)"
		eval pkg_new_version=\$${pkgupper}_VERSION

		if [ "$pkg_old_version" != "$pkg_new_version" ]
		then
			echo "$pkg: $pkg_old_version --> $pkg_new_version"
		fi
		)
	done | sort
esac
