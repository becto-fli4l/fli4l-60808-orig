#!/bin/bash

# $1 = package
# $2 = source files.txt
# $3 = target files.txt
process_filestxt()
{
	local pkg=$1 f=$2 filestxt=$3 sortfiles=$4 sortfiles_k2=$5 fbrpkg license
	local pkgdir="$(dirname "$filestxt")"
	local legaldir="$pkgdir/opt/usr/share/legal-info/$pkg"
	local manifest="$legaldir/manifest.csv" host_manifest="$legaldir/host-manifest.csv"
	mkdir -p "$legaldir"
	> "$manifest"
	> "$host_manifest"

	while read pkgfile
	do
		[ -f "$binroot/$pkgfile" ] || continue
		for fbrpkg in $(sed -n "s,^\([^ ]\+\) ${pkgfile//,/\\,}$,\1,p" $br_wcout/sandbox/*/target.installed 2>/dev/null | sort -u)
		do
			case $fbrpkg in
			host-*)
				for license in $(find "$legal_info_source_dir/host-licenses/${fbrpkg#host-}" -type f 2>/dev/null)
				do
					license=${license#$legal_info_source_dir/}
					echo "d opt/usr/share/legal-info/$license"
				done
				grep "^\"${fbrpkg#host-}\"" "$legal_info_source_dir/host-manifest.csv" >> "$host_manifest"
				;;
			*)
				for license in $(find "$legal_info_source_dir/licenses/$fbrpkg" -type f 2>/dev/null)
				do
					license=${license#$legal_info_source_dir/}
					echo "d opt/usr/share/legal-info/$license"
				done
				grep "^\"$fbrpkg\"" "$legal_info_source_dir/manifest.csv" >> "$manifest"
				;;
			esac
		done
	done < <(sed -n "s,^. \($fbr_arch::\)\?opt/,,p" $f $filestxt 2>/dev/null) > "$filestxt.tmp"

	if [ -s "$manifest" ]
	then
		echo "d opt/usr/share/legal-info/$pkg/manifest.csv" >> "$filestxt"
		flock $sortfiles echo "$manifest" >> $sortfiles
	else
		rm -f "$manifest" 2>/dev/null
		if [ "$update_svn" = yes ]
		then
			flock $binroot svn rm --force "$manifest" 2>/dev/null
		fi
	fi
	if [ -s "$host_manifest" ]
	then
		echo "d opt/usr/share/legal-info/$pkg/host-manifest.csv" >> "$filestxt"
		flock $sortfiles echo "$host_manifest" >> $sortfiles
	else
		rm -f "$host_manifest" 2>/dev/null
		if [ "$update_svn" = yes ]
		then
			flock $binroot svn rm --force "$host_manifest" 2>/dev/null
		fi
	fi
	if [ ! -s "$manifest" -a ! -s "$host_manifest" ]
	then
		rmdir "$pkgdir/opt/usr/share/legal-info/$pkg" 2>/dev/null
		rmdir "$pkgdir/opt/usr/share/legal-info" 2>/dev/null
		rmdir "$pkgdir/opt/usr/share" 2>/dev/null
		rmdir "$pkgdir/opt/usr" 2>/dev/null

		if [ "$update_svn" = yes ]
		then
			[ -d "$pkgdir/opt/usr/share/legal-info/$pkg" ] ||
				flock $binroot svn rm --force "$pkgdir/opt/usr/share/legal-info/$pkg" 2>/dev/null
			[ -d "$pkgdir/opt/usr/share/legal-info" ] ||
				flock $binroot svn rm --force "$pkgdir/opt/usr/share/legal-info" 2>/dev/null
			[ -d "$pkgdir/opt/usr/share" ] ||
				flock $binroot svn rm --force "$pkgdir/opt/usr/share" 2>/dev/null
			[ -d "$pkgdir/opt/usr" ] ||
				flock $binroot svn rm --force "$pkgdir/opt/usr" 2>/dev/null
		fi
	fi

	# FFL-2200: remove obsolete legal-info
	if [ -d "$pkgdir/usr/share/legal-info" ]
	then
		rm -f "$pkgdir/usr/share/legal-info/$pkg/manifest.csv" 2>/dev/null
		rm -f "$pkgdir/usr/share/legal-info/$pkg/host-manifest.csv" 2>/dev/null
		rmdir "$pkgdir/usr/share/legal-info/$pkg" 2>/dev/null
		rmdir "$pkgdir/usr/share/legal-info" 2>/dev/null
		rmdir "$pkgdir/usr/share" 2>/dev/null
		rmdir "$pkgdir/usr" 2>/dev/null

		if [ "$update_svn" = yes ]
		then
			flock $binroot svn rm --force "$pkgdir/usr/share/legal-info/$pkg/manifest.csv" 2>/dev/null
			flock $binroot svn rm --force "$pkgdir/usr/share/legal-info/$pkg/host-manifest.csv" 2>/dev/null
			[ -d "$pkgdir/usr/share/legal-info/$pkg" ] ||
				flock $binroot svn rm --force "$pkgdir/usr/share/legal-info/$pkg" 2>/dev/null
			[ -d "$pkgdir/usr/share/legal-info" ] ||
				flock $binroot svn rm --force "$pkgdir/usr/share/legal-info" 2>/dev/null
			[ -d "$pkgdir/usr/share" ] ||
				flock $binroot svn rm --force "$pkgdir/usr/share" 2>/dev/null
			[ -d "$pkgdir/usr" ] ||
				flock $binroot svn rm --force "$pkgdir/usr" 2>/dev/null
		fi

		sed -i '/^. opt\/files\/usr\/share\/legal-info\//d' "$filestxt"
	fi

	cat "$filestxt.tmp" >> "$filestxt"
	rm -f "$filestxt.tmp"
	if [ -s "$filestxt" ]
	then
		flock $sortfiles_k2 echo "$filestxt" >> $sortfiles_k2
	else
		rm -f "$filestxt" 2>/dev/null
		rmdir "$(dirname "$filestxt")" 2>/dev/null

		if [ "$update_svn" = yes ]
		then
			flock $binroot svn rm --force "$filestxt" 2>/dev/null
			flock $binroot svn rm --force "$(dirname "$filestxt")" 2>/dev/null
		fi
	fi

	spinner_show
}

add_legal_info()
{
	legal_info_source_dir=$br_wcout/legal-info
	if [ -d $legal_info_source_dir ]
	then
		message "  Adding legal info (licenses)..." "" "-n"
		spinner_init

		legal_info_target_dir=$binroot/usr/share/legal-info
		rm -rf $legal_info_target_dir
		mkdir -p $legal_info_target_dir
		find $legal_info_source_dir -mindepth 1 -maxdepth 1 \! \( -name sources -or -name host-sources \) -exec cp -r {} $legal_info_target_dir \;

		local f pkg packages=
		local srcroot="$svnroot/src"
		sed -i "/^. opt\/usr\/share\/legal-info\//d" "$binroot"/packages/*/files.txt 2>/dev/null

		for f in "$srcroot"/packages/*/files.txt "$binroot"/packages/*/files.txt
		do
			[ -f "$f" ] || continue
			pkg="$(basename "$(dirname "$f")")"
			if ! echo "$packages" | grep -q " $pkg\(\$\| \)"
			then
				packages+=" $pkg"
			fi
		done

		local sortfiles=$(mktemp -q "sortfiles.XXXXXX")
		local sortfiles_k2=$(mktemp -q "sortfiles_k2.XXXXXX")
		for pkg in $packages
		do
			( process_filestxt $pkg \
				"$srcroot/packages/$pkg/files.txt" \
				"$binroot/packages/$pkg/files.txt" \
				"$sortfiles" "$sortfiles_k2" ) &
		done
		wait

		local f
		for f in $(cat $sortfiles)
		do
			spinner_show
			LC_ALL=C sort -u -o $f $f
		done
		for f in $(cat $sortfiles_k2)
		do
			spinner_show
			LC_ALL=C sort -k 2 -u -o $f $f
		done

		rm -f "$sortfiles" "$sortfiles_k2"
		spinner_done
	fi
}

update_plugin_messages+="      --update-legal-info      update legal information
"

if [ "$update_legal_info" = yes ]
then
	update_plugins+=" add_legal_info"
fi
