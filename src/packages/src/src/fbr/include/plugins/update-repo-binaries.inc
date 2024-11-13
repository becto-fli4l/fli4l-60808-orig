#!/bin/bash

update_entry()
{
	local src=$1 tgt=$2
	tgtdir=$(dirname "$tgt")
	if [ -L "$tgt" -o -L "$src" ]
	then
		if [ -L "$tgt" -a -L "$src" ]
		then
			[ "$(readlink "$tgt")" = "$(readlink "$src")" ] && return
		#else: always overwrite target with source
		fi
	elif [ -f "$tgt" -a -f "$src" ]
	then
		cmp -s "$src" "$tgt" && return
	fi

	if [ -e "$tgt" ] && [ ! -d "$tgt" -o ! -d "$src" ]
	then
		rm -rf "$tgt"
	fi

	if [ -d "$src" ]
	then
		[ -e "$tgt" ] || message "  ${tgt#$binroot}"
		mkdir -p "$tgt"
	else
		message "  ${tgt#$binroot}"
		mkdir -p "$tgtdir"
		cp -aT "$src" "$tgt"
	fi
}

command=$1
shift

while true
do
	case $1 in
	--*)
		name=${1#--}
		name=${name//-/_}
		eval $name=yes
		;;
	*)
		break
		;;
	esac
	shift
done

update_plugins=
update_plugin_messages=
for f in "$fbr_root/include/plugins/update"/*.inc
do
	[ -f "$f" ] || continue
	. "$f"
done

case "$command" in
help)
	message "  $progname update-repo-binaries [options...] <path-to-fli4l-SVN-working-copy>" yellow
	message "    updates all binaries of a working copy with binaries from the FBR"
	message "    possible options:"
	message "      --delete-missing         delete files from checkout missing in the FBR"
	message "      --update-svn             update SVN state of checkout"
	[ -n "$update_plugin_messages" ] &&
		message "$update_plugin_messages" "" -n
	;;

action)
	message "Updating binaries in SVN working copy" standout
	svnroot="$1"

	[ -n "$svnroot" ] || \
		error "Directory of fli4l SVN working copy is missing" 100

	[ -d "$svnroot" ] || \
		error "$svnroot does not point to a fli4l SVN working copy" 101

	pkgroot="$svnroot/src/packages"
	[ -d "$pkgroot" ] || error "$svnroot/src/packages does not exist" 102

	binroot="$svnroot/bin/$fbr_arch"
	mkdir -p "$binroot"

	message "Copying new and updating existing files..."
	for dir in bin etc lib libexec sbin srv usr windows
	do
		find "$br_wcout/target/$dir" -mindepth 1 2>/dev/null | while read src
		do
			tgt="$binroot/${src#$br_wcout/target/}"
			update_entry "$src" "$tgt"
		done
	done
	find "$br_wcout/images/" -mindepth 1 2>/dev/null | while read src
	do
		tgt="$binroot/img/${src#$br_wcout/images/}"
		update_entry "$src" "$tgt"
	done

	if [ "$delete_missing" = yes ]
	then
		message "Deleting obsolete files..."
		for dir in bin boot etc lib libexec sbin srv usr windows
		do
			find "$binroot/$dir" -mindepth 1 -type d 2>/dev/null | while read tgt
			do
				src="$br_wcout/target/${tgt#$binroot/}"
				if [ -e "$tgt" -a ! -e "$src" ]
				then
					message "  ${tgt#$binroot}"
					rm -rf "$tgt"
				fi
			done
			find "$binroot/$dir" -mindepth 1 ! -type d 2>/dev/null | while read tgt
			do
				src="$br_wcout/target/${tgt#$binroot/}"
				if [ -e "$tgt" -a ! -e "$src" ]
				then
					message "  ${tgt#$binroot}"
					rm -f "$tgt"
				fi
			done
		done
		find "$binroot/img" -mindepth 1 -type d 2>/dev/null | while read tgt
		do
			src="$br_wcout/images/${tgt#$binroot/img/}"
			if [ -e "$tgt" -a ! -e "$src" ]
			then
				message "  ${tgt#$binroot}"
				rm -rf "$tgt"
			fi
		done
		find "$binroot/img" -mindepth 1 ! -type d 2>/dev/null | while read tgt
		do
			src="$br_wcout/images/${tgt#$binroot/img/}"
			if [ -e "$tgt" -a ! -e "$src" ]
			then
				message "  ${tgt#$binroot}"
				rm -f "$tgt"
			fi
		done
	fi

	if [ -n "$update_plugins" ]
	then
		message "Do post-processing..."
		for p in $update_plugins
		do
			$p $3
		done
		message "Done."
	fi

	if [ "$update_svn" = yes ]
	then
		message "Updating SVN checkout..."
		svn cleanup "$svnroot/bin"
		svn status "$binroot" | sed -n 's/^~//p' | while read f
		do
			mv "$f" "$f.temporarily_moved"
			svn rm --force "$f" >/dev/null
			mv "$f.temporarily_moved" "$f"
			svn add "$f" >/dev/null
		done
		svn add --force --depth=empty "$svnroot/bin" >/dev/null
		svn add --force --no-ignore "$binroot" >/dev/null
		svn status "$binroot" | sed -n 's/^!...... \(.*\)/\1@/p' | xargs -r svn rm --force >/dev/null
	fi
	;;
esac
