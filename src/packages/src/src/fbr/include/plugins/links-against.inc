#!/bin/bash

case "$1" in
help)
	message "  $progname links-against <library-soname>" yellow
	message "    prints all files in output/target which link against given library"
	;;

action)
	message "Files linking against $2" standout

	while read p f
	do
		ff="$br_wcout/target/$f"
		[ -h "$ff" -o ! -x "$ff" ] && continue
		readelf -d "$ff" 2>/dev/null \
			| grep -q "(NEEDED).*\[$2[^]]*\]" \
				&& echo "$p $f"
	done < <(cat $br_wcout/sandbox/*/target.installed | sort -u)
	;;
esac
