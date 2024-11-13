#!/bin/bash

fbr_revdeps=$br_wcdir/.reverse-deps
fbr_nestedrevdeps=$br_wcdir/.reverse-deps.nested
fbr_configopts=$br_wcdir/.config-opts
fbr_configopts_old=$br_wcdir/.config-opts.old
fbr_safe_package_files="^/\(Config\.in[^[:space:]]*\|package/Config\.in[^[:space:]]*\)$"
fbr_dont_copy="^/\(\.config-[^[:space:]]\+\|${fbr_own_packages//./\\.}/\.config-[^[:space:]]\+\|output\)$"
fbr_dont_delete="^/\(\.\.config.tmp\|\.config-[^[:space:]]\+\.old\|\.reverse-deps\(\.new\|\.nested\)\?\|\.config-opts\(\.old\)\?\|dl\|output\|${fbr_own_packages_path//./\\.}\|${fbr_own_packages//./\\.}/Config.in[^[:space:]]*\|${fbr_own_packages//./\\.}/${fbr_own_packages//./\\.}.mk\|\.hashes\)$"

fbr_to_uninstall=" "
fbr_to_process=
fbr_hash_needed=
declare -A fbr_uninstall_messages

# $1 = path to directory
# $2 = variable receiving the package makefile
# exit code: 0 = directory represents a package
# output: $$2 = name of package's makefile
is_package()
{
	[ -d "$1" ] || return 1
	local pn="${1##*/}"
	if [ -f "$1/$pn.mk" ]
	then
		[ -n "$2" ] && eval $2=\"\$pn.mk\"
		return 0
	elif [ -f "$1/${pn/-/_}.mk" ]
	then
		[ -n "$2" ] && eval $2=\"\$\{pn/-/_}.mk\"
		return 0
	elif [ -f "$1/${pn/_/-}.mk" ]
	then
		[ -n "$2" ] && eval $2=\"\$\{pn/_/-}.mk\"
		return 0
	else
		return 1
	fi
}

# $1 = package to uninstall
# $2 = reason
fbr_uninstall_package()
{
	[ "$fbr_to_uninstall" = "root" ] && return 0
	if [ -z "$1" ]
	then
		[ -n "$2" ] && fbr_uninstall_messages["root"]="Uninstalling everything because $2"
		fbr_to_uninstall="root"
	elif ! echo "$fbr_to_uninstall" | grep -q " $1 "
	then
		[ -n "$2" ] && fbr_uninstall_messages["$1"]="Uninstalling $1 because $2"
		fbr_to_uninstall+="$1 "
	fi
}

fbr_print_uninstall_messages()
{
	local pkg message
	for pkg in "${!fbr_uninstall_messages[@]}"
	do
		echo "${fbr_uninstall_messages["$pkg"]}"
		unset fbr_uninstall_messages["$pkg"]
	done
}

fbr_complete_uninstall_packages()
{
	[ "$fbr_to_uninstall" = "root" ] && return 0
	local pkg
	local -A reason

	local orig_pkgs="$fbr_to_uninstall"

	local fbr_uninst=
	while [ "$fbr_uninst" != "$fbr_to_uninstall" ]
	do
		fbr_uninst=$fbr_to_uninstall
		fbr_to_uninstall=" "
		for pkg in $fbr_uninst
		do
			fbr_uninstall_package "$pkg"
			local revdep
			for revdep in $(sed -n "s/^$pkg://p" $fbr_revdeps)
			do
				reason["$revdep"]="it depends on $pkg which needs to be uninstalled"
				fbr_uninstall_package "$revdep"
			done
			# only uninstall child packages if parent package has been actually
			# modified, not only because a sibling has changed
			if echo "$orig_pkgs" | grep -q " $pkg "
			then
				local child
				for child in $(sed -n "s/^$pkg://p" $fbr_nestedrevdeps)
				do
					reason["$child"]="it is a child of $pkg which needs to be uninstalled"
					fbr_uninstall_package "$child"
					orig_pkgs+="$child "
				done
			fi
		done
	done

	fbr_to_uninstall=" "
	for pkg in $fbr_uninst
	do
		if grep -q "^$pkg:" $fbr_revdeps
		then
			fbr_uninstall_package "$pkg" "${reason["$pkg"]}"
		fi
	done
}

# $1 = package
# $2 = target base
# $3 = relative path
# $3 = reason
fbr_rebuild_package()
{
	local pkg=$1 tgtbase=$2 relpath=$3 reason=$4

	case $relpath in
	/board/*)
		for d in "$tgtbase"/package/boot/* "$tgtbase"/linux
		do
			[ -d "$d" ] || continue
			pkg=$(basename $d)
			fbr_uninstall_package "$pkg" "$reason"
			fbr_uninstall_package "host-$pkg" "$reason"
		done
		;;
	/build/*|/configs/*|/docs/*)
		;;
	*)
		fbr_uninstall_package "$pkg" "$reason"
		fbr_uninstall_package "host-$pkg" "$reason"
		;;
	esac
}

fbr_delete()
{
	local pkg=$1
	local srcbase=$2
	local tgtbase=$3
	local relpath=$4
	local srccfgoptfile=$5
	local tgtcfgoptfile=$6
	local nestedrevdeps=$7
	local tgtpkg=$8
	local reason=${9:-"$relpath has been removed"}

	echo "$relpath" | grep -q "$fbr_dont_delete" && return 0

	if [ -d "$tgtbase$relpath" ]
	then
		if [ $tgtpkg -ne 0 ]
		then
			fbr_scan "${relpath##*/}" "$srcbase" "$tgtbase" "$relpath" "$srccfgoptfile" "$tgtcfgoptfile" "$nestedrevdeps"
		else
			fbr_scan "$pkg" "$srcbase" "$tgtbase" "$relpath" "$srccfgoptfile" "$tgtcfgoptfile" "$nestedrevdeps"
		fi
	fi

	fbr_to_process+=" / $tgtbase$relpath"
	if [ $tgtpkg -eq 0 ]
	then
		fbr_rebuild_package "$pkg" "$tgtbase" "$relpath" "$reason"
	fi
}

fbr_add()
{
	local pkg=$1
	local srcbase=$2
	local tgtbase=$3
	local relpath=$4
	local srccfgoptfile=$5
	local tgtcfgoptfile=$6
	local nestedrevdeps=$7
	local srcpkg=$8
	local reason=${9:-"$relpath has been added"}

	echo "$relpath" | grep -q "$fbr_dont_copy" && return 0

	fbr_to_process+=" $srcbase$relpath $tgtbase$relpath"
	if [ $srcpkg -eq 0 ]
	then
		fbr_rebuild_package "$pkg" "$tgtbase" "$relpath" "$reason"
	fi

	if [ -d "$srcbase$relpath" ]
	then
		if [ $srcpkg -ne 0 ]
		then
			fbr_scan "${relpath##*/}" "$srcbase" "$tgtbase" "$relpath" "$srccfgoptfile" "$tgtcfgoptfile" "$nestedrevdeps"
		else
			fbr_scan "$pkg" "$srcbase" "$tgtbase" "$relpath" "$srccfgoptfile" "$tgtcfgoptfile" "$nestedrevdeps"
		fi
	fi
}

fbr_replace()
{
	local pkg=$1
	local srcbase=$2
	local tgtbase=$3
	local relpath=$4
	local srccfgoptfile=$5
	local tgtcfgoptfile=$6
	local nestedrevdeps=$7
	local srcpkg=$8
	local tgtpkg=$9

	fbr_delete "$pkg" "$srcbase" "$tgtbase" "$relpath" "$srccfgoptfile" "$tgtcfgoptfile" "$nestedrevdeps" "$tgtpkg" "$relpath has been replaced"
	fbr_add "$pkg" "$srcbase" "$tgtbase" "$relpath" "$srccfgoptfile" "$tgtcfgoptfile" "$nestedrevdeps" "$srcpkg" "$relpath is a replacement"
}

fbr_modify()
{
	local pkg=$1
	local srcbase=$2
	local tgtbase=$3
	local relpath=$4
	local srccfgoptfile=$5
	local tgtcfgoptfile=$6
	local nestedrevdeps=$7
	local reason

	echo "$relpath" | grep -q "$fbr_dont_copy" && return 0

	local elem=${relpath##*/}
	local diff=
	if [ -x "$srcbase$relpath" -a ! -x "$tgtbase$relpath" ] ||
		[ ! -x "$srcbase$relpath" -a -x "$tgtbase$relpath" ]
	then
		reason="$relpath has changed executable state"
		diff=1
	elif [ \( "$srcbase$relpath" -nt "$tgtbase$relpath" -o \
		"$tgtbase$relpath" -nt "$srcbase$relpath" \) ] &&
		! cmp -s "$srcbase$relpath" "$tgtbase$relpath"
	then
		reason="$relpath has been modified"
		diff=1
	fi

	if [ "$elem" = "$makefile" ] &&
		grep -q "SITE_METHOD *= *local$" "$srcbase$relpath" &&
		grep -q "SITE *= *\$(FLI4L_SRCDIR)" "$srcbase$relpath"
	then
		local srcdir=$(realpath "$fbr_root/../fli4l/$(sed -n "s/^.*SITE *= *\$(FLI4L_SRCDIR)\///p" "$srcbase$relpath")")
		if [ -d "$srcdir" ]
		then
			local rehash=
			local hashfile="$br_wcdir/.hashes/${elem%.mk}"
			if [ -f "$hashfile" ]
			then
				local oldhash=$(cat "$hashfile")
				local newhash=$(find "$srcdir/" -type f -print0 | sort -z | xargs -0 sha256sum | sha256sum | cut -d ' ' -f 1)
				if [ "$oldhash" != "$newhash" ]
				then
					: ${reason:="computed directory hash differs from stored directory hash"}
					rehash=1
				fi
			else
				: ${reason:="stored directory hash does not exist"}
				rehash=1
			fi

			if [ -n "$rehash" ]
			then
				diff=1
				fbr_hash_needed+=" $tgtbase$relpath"
			fi
		fi
	fi

	if [ -n "$diff" ]
	then
		fbr_to_process+=" $srcbase$relpath $tgtbase$relpath"

		echo "$relpath" | grep -q "$fbr_safe_package_files" && return 0

		if [ "$relpath" != "/package/conflict.resolver" ] ||
			[ $conflict_changes -ne 0 ]
		then
			fbr_rebuild_package "$pkg" "$tgtbase" "$relpath" "$reason"
		fi
	fi
}

# $1 = package
# $2 = source root
# $3 = target root
# $4 = path relative to root(s)
# $5 = output file for new configuration options
# $6 = output file for old configuration options
# $7 = file containing additional reverse dependencies for the target
fbr_scan()
{
	local pkg=$1
	local srcbase=$2
	local tgtbase=$3
	local relpath=$4
	local srccfgoptfile=$5
	local tgtcfgoptfile=$6
	local nestedrevdeps=$7

	local srcregex=$(echo "$srcbase" | sed 's/[/.]/\\&/g')
	local tgtregex=$(echo "$tgtbase" | sed 's/[/.]/\\&/g')

	# makes makefile local per hierarchy level
	local makefile=
	is_package "$srcbase$relpath" makefile

	local elem
	while read elem
	do
		spinner_show

		echo "$elem" | grep -q "$fbr_dont_copy" && continue

		local tgtpkg
		local srcpkg
		is_package "$tgtbase$elem" && tgtpkg=1 || tgtpkg=0
		is_package "$srcbase$elem" && srcpkg=1 || srcpkg=0

		if [ $srcpkg -ne 0 ]
		then
			echo "${elem##*/}: "$(sed -n "s/^config[[:space:]]\+//p" $srcbase$elem/Config.in $srcbase$elem/Config.in.host 2>/dev/null) >> $srccfgoptfile
		fi

		if [ $tgtpkg -ne 0 ]
		then
			echo "${elem##*/}: "$(sed -n "s/^config[[:space:]]\+//p" $tgtbase$elem/Config.in $tgtbase$elem/Config.in.host 2>/dev/null) >> $tgtcfgoptfile
		fi

		if [ -L "$srcbase$elem" -o -L "$tgtbase$elem" ]
		then
			# special case: at least one side is a symlink
			local srclnktgt= tgtlnktgt=
			[ -L "$srcbase$elem" ] && srclnktgt=$(readlink "$srcbase$elem")
			[ -L "$tgtbase$elem" ] && tgtlnktgt=$(readlink "$tgtbase$elem")
			if [ "$srclnktgt" != "$tgtlnktgt" ]
			then
				[ -e "$tgtbase$elem" -o -L "$tgtbase$elem" ] &&
					fbr_delete "$pkg" "$srcbase" "$tgtbase" "$elem" "$srccfgoptfile" "$tgtcfgoptfile" "$nestedrevdeps" "$tgtpkg" "$relpath has been replaced by a symlink or is a replaced symlink"
				[ -e "$srcbase$elem" -o -L "$srcbase$elem" ] &&
					fbr_add "$pkg" "$srcbase" "$tgtbase" "$elem" "$srccfgoptfile" "$tgtcfgoptfile" "$nestedrevdeps" "$srcpkg" "$relpath replaces a symlink or is a replacing symlink"
			fi
		else
			# from here, no symlinks need to be handled
			if [ -e "$srcbase$elem" -a ! -e "$tgtbase$elem" ]
			then
				fbr_add "$pkg" "$srcbase" "$tgtbase" "$elem" "$srccfgoptfile" "$tgtcfgoptfile" "$nestedrevdeps" "$srcpkg"
			elif [ ! -e "$srcbase$elem" -a -e "$tgtbase$elem" ]
			then
				fbr_delete "$pkg" "$srcbase" "$tgtbase" "$elem" "$srccfgoptfile" "$tgtcfgoptfile" "$nestedrevdeps" "$tgtpkg"
			elif [ -f "$tgtbase$elem" ]
			then
				if [ -f "$srcbase$elem" ]
				then
					fbr_modify "$pkg" "$srcbase" "$tgtbase" "$elem" "$srccfgoptfile" "$tgtcfgoptfile" "$nestedrevdeps"
				else
					fbr_replace "$pkg" "$srcbase" "$tgtbase" "$elem" "$srccfgoptfile" "$tgtcfgoptfile" "$nestedrevdeps" "$srcpkg" "$tgtpkg"
				fi
			elif [ -d "$tgtbase$elem" ]
			then
				if [ $tgtpkg -ne 0 ]
				then
					[ -n "$pkg" ] && echo "$pkg: ${elem##*/}" >> $nestedrevdeps
					fbr_scan "${elem##*/}" "$srcbase" "$tgtbase" "$elem" "$srccfgoptfile" "$tgtcfgoptfile" "$nestedrevdeps"
				else
					fbr_scan "$pkg" "$srcbase" "$tgtbase" "$elem" "$srccfgoptfile" "$tgtcfgoptfile" "$nestedrevdeps"
				fi

				if [ ! -d "$srcbase$elem" ] || [ $tgtpkg -ne 0 -a $srcpkg -eq 0 ]
				then
					fbr_replace "$pkg" "$srcbase" "$tgtbase" "$elem" "$srccfgoptfile" "$tgtcfgoptfile" "$nestedrevdeps" "$srcpkg" "$tgtpkg"
				fi
			else
				fbr_replace "$pkg" "$srcbase" "$tgtbase" "$elem" "$srccfgoptfile" "$tgtcfgoptfile" "$nestedrevdeps" "$srcpkg" "$tgtpkg"
			fi
		fi
	done < <(
		(
			[ -d "$srcbase$relpath" ] && find "$srcbase$relpath/" -mindepth 1 -maxdepth 1 | sed "s,^$srcregex,,"
			[ -d "$tgtbase$relpath" ] && find "$tgtbase$relpath/" -mindepth 1 -maxdepth 1 | sed "s,^$tgtregex,,"
		) | LC_COLLATE=C sort -u
	)
}

fbr_prepare_own_packages()
{
	# package/own/Config.in
	echo "menu \"$fbr_own_packages_menu\"" > $fbr_own_wc_dir/Config.in
	for f in $fbr_own_wc_dir/*/Config.in
	do
		[ -e $f ] || continue
		echo "source \"${fbr_own_packages_path}${f#$fbr_own_wc_dir}\"" >> $fbr_own_wc_dir/Config.in
	done
	echo "endmenu" >> $fbr_own_wc_dir/Config.in

	# package/own/own.mk
	echo "include \$(sort \$(wildcard $fbr_own_packages_path/*/*.mk))" > $fbr_own_wc_dir/$fbr_own_packages.mk
}

fbr_analyse()
{
	message "Analysing FBR working copy..." standout "-n"

	> $fbr_nestedrevdeps
	> $fbr_configopts
	> $fbr_configopts_old

	if [ ! -f $fbr_revdeps ]
	then
		fbr_uninstall_package "" "reverse dependencies are missing"
		> $fbr_revdeps
	fi

	mkdir -p $br_wcdir/package
	local conflict_changes=$(diff -c $br_wcdir/package/conflict.resolver $br_dir/package/conflict.resolver 2>/dev/null | grep -c "^! ")

	spinner_init

	[ -f $br_wcdir/Config.in ] && sed -i '$d' $br_wcdir/Config.in
	fbr_scan "" $br_dir $br_wcdir "" $fbr_configopts $fbr_configopts_old $fbr_nestedrevdeps

	rm -rf $fbr_own_wc_dir
	cp -a $fbr_owndir $fbr_own_wc_dir
	fbr_prepare_own_packages
	fbr_scan own $fbr_own_wc_dir $br_wcdir/$fbr_own_packages_path "" $fbr_configopts $fbr_configopts_old $fbr_nestedrevdeps

	spinner_done
}

fbr_uninstall()
{
	if [ "$fbr_to_uninstall" = "root" ]
	then
		fbr_print_uninstall_messages
		message "Deleting FBR output files..." standout "-n"
		[ -d $br_wcout ] && rm -rf $br_wcout
		mkdir -p $br_wcout
		message " done."
	else
		fbr_complete_uninstall_packages
		fbr_print_uninstall_messages
		if [ -n "${fbr_to_uninstall# }" ]
		then
			message "Uninstalling packages..." standout
			make_targets $(printf '%s\n' $fbr_to_uninstall | sort -u | sed -e 's/.*/&-uninstall &-dirclean/')
			fbr_to_uninstall=
		fi
	fi
}

fbr_update()
{
	# uninstall packages
	fbr_uninstall

	local elem
	local num
	local i

	if [ -n "${fbr_to_process# }" ]
	then
		message "Updating FBR files/directories..." standout "-n"
		num=$(echo $fbr_to_process | wc -w)
		i=0
		set -- $fbr_to_process
		while [ $# -gt 0 ]
		do
			local src=$1
			local tgt=$2
			shift 2
			percentage_show $i $num
			rm -rf "$tgt"
			if [ "$src" != "/" ]
			then
				mkdir -p "$(dirname "$tgt")"
				cp -a "$src" "$tgt"
			fi
			i=$((i+2))
		done
		percentage_done
		fbr_to_process=
	fi

	# integrate own packages into Config.in
	echo "source \"$fbr_own_packages_path/Config.in\"" >> $br_wcdir/Config.in
}

# $1 = package
# $2 = target root
# $3 = path relative to root(s)
fbr_update_hashes()
{
	message "Updating hashes..." standout "-n"

	mkdir -p "$br_wcdir/.hashes"
	local num=$(echo "$fbr_hash_needed" | wc -w)
	local i=0
	local elem
	for elem in $fbr_hash_needed
	do
		percentage_show $i $num
		i=$((i+1))

		local makefile=$(basename $elem)
		local srcdir=$(realpath "$fbr_root/../fli4l/$(sed -n "s/^.*SITE *= *\$(FLI4L_SRCDIR)\///p" "$elem")")
		if [ -d "$srcdir" ]
		then
			local hashfile="$br_wcdir/.hashes/${makefile%.mk}"
			local hash=$(find "$srcdir/" -type f -print0 | sort -z | xargs -0 sha256sum | sha256sum | cut -d ' ' -f 1)
			echo $hash > "$hashfile"
		fi
	done

	percentage_done
}

fbr_compare_configuration()
{
	message "Comparing FBR configuration..." standout "-n"
	local cfgopts=$(diff -U 0 <(cat $br_config $fbr_own_packages_config 2>/dev/null) $br_wc_config 2>/dev/null |
		grep "^[+-][^# ]*=.*$" | sed -e 's/^[+-]\([^#= ]*\).*/\1/' |
		sort -u)

	local num=$(echo "$cfgopts" | wc -w)
	local i=0
	local cfgopt
	for cfgopt in $cfgopts
	do
		percentage_show $i $num
		i=$((i+1))
		case $cfgopt in
			BR2_JLEVEL)
				# parallelization is controlled by "J" environment variable
				continue;;
			BR2_HOST*)
				# different host configurations shall not cause rebuilds
				continue;;
		esac

		local reason="configuration option $cfgopt changed"
		local pkg=$(grep "\<$cfgopt\>" $fbr_configopts | sed -e 's/^\([^:]*\):.*/\1/')
		if [ ! -n "$pkg" ]
		then
			pkg=$(grep "\<$cfgopt\>" $fbr_configopts_old | sed -e 's/^\([^:]*\):.*/\1/')
		fi
		if [ -n "$pkg" ]
		then
			fbr_uninstall_package "$pkg" "$reason"
			fbr_uninstall_package "host-$pkg" "$reason"
		else
			fbr_uninstall_package "" "$reason"
		fi
	done
	rm -f $fbr_configopts $fbr_configopts_old
	percentage_done
}

fbr_update_configuration()
{
	message "Updating FBR configuration..." standout "-n"
	cat $br_config $fbr_own_packages_config > $br_wc_config 2>/dev/null
	percentage_done
}

fbr_update_reverse_deps()
{
	message "Computing new package dependencies..." standout "-n"
	make --no-print-directory -C $br_wcdir BR2_CONFIG=$br_wc_config \
		BR2_DL_DIR=$fbr_dldir show-reverse-deps-complete > $fbr_revdeps.new
	percentage_done

	message "Searching for packages with new dependencies..." standout "-n"

	# uninstall all packages whose dependencies have changed
	local pkgdiff=$(diff -u $fbr_revdeps $fbr_revdeps.new | sed -n 's/^+\([^ ]\+\):.*$/\1/p')
	local num=$(echo "$pkgdiff" | wc -w)
	local i=0
	local pkg
	for pkg in $pkgdiff
	do
		percentage_show $i $num
		i=$((i+1))

		local oldrevdeps=$(diff -u $fbr_revdeps $fbr_revdeps.new | sed -n "s/^-$pkg:\(.*\)$/\1 /p")
		local newrevdeps=$(diff -u $fbr_revdeps $fbr_revdeps.new | sed -n "s/^+$pkg:\(.*\)$/\1 /p")
		for dependee in $newrevdeps
		do
			if ! echo "$oldrevdeps" | grep -q " $dependee "
			then
				fbr_uninstall_package "$dependee" "it now depends on $pkg"
			fi
		done
	done
	percentage_done

	mv -f $fbr_revdeps.new $fbr_revdeps
	fbr_uninstall
}

mkdir -p $br_wcout

if [ "$fbr_nocheck" != y ]
then
	fbr_analyse
	fbr_compare_configuration
	fbr_update
	fbr_update_configuration
	fbr_update_reverse_deps
	fbr_update_hashes
fi
