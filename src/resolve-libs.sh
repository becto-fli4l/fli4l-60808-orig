#!/bin/bash
# $1 = ELF file
# $2... = base directories containing /lib and /usr/lib directories

# library directories to search (must coincide with uClibc implementation!)
libdirs="lib usr/lib"

## for later use of $SED (MacOS vs. Linux sed)
type -p gsed >/dev/null && SED=gsed || SED=sed

# $1 = string
# result = the same string, but usable in a regex
escape_regex()
{
	echo "$1" | $SED -e 's/\./\\./g;s/\[/\\[/g;s/\]/\\]/g;s/\*/\\*/g;s/\^/\\^/g;s/\$/\\$/g'
}

get_all_packages()
{
	local pkgdir
	for pkgdir in packages/*
	do
		[ -f "$pkgdir/files.txt" ] || continue
		echo "${pkgdir#packages/}"
	done
}

# $1 = ELF file
# $2 = property to filter
readelf_filter()
{
	set -o pipefail
	readelf -d "$1" 2>/dev/null | grep "($2)" | $SED -e 's/^.*\[\([^]]\+\)\]$/\1/'
	local res=$?
	set +o pipefail
	return $res
}

# $1 = root directory containing libraries
# $2 = subdirectory
# $3... = packages
read_sonames()
{
	local binroot="$1"
	local subdir="$2"
	shift 2
	[ -n "$1" ] || set -- $(get_all_packages)

	local pkg
	for pkg
	do
		local arch lib
		while read arch lib
		do
			# check if architecture is suitable
			arch=${arch#arch=}
			[ -z "$arch" -o "$arch" = "$ARCH" ] || continue

			# check if file exists
			path="$binroot/$lib"
			[ ! -f "$path" ] && path="packages/$pkg/opt/$lib"
			[ ! -f "$path" ] && continue

			# check if file is an ELF object
			readelf -h "$path" >/dev/null 2>&1 || continue

			# determine SONAME
			local soname=$(readelf_filter "$path" "SONAME")
			[ -n "$soname" ] || soname=$(basename "$lib")
			echo "$pkg $soname $path"
		done < <($SED -n "s#^[bB] \(\([^:]\+\)::\)\?opt/$subdir/\([^/]\+\)\$#arch=\2 $subdir/\3#p" "packages/$pkg/files.txt" "$binroot/packages/$pkg/files.txt" 2>/dev/null)
	done
}

# $1 = library soname
# $2... = packages
map_soname()
{
	local soname="$1"
	shift 1
	[ -n "$1" ] || set -- $(get_all_packages)

	local expr="\($1"
	shift
	local pkg
	for pkg
	do
		expr+="\|$pkg"
	done
	expr+="\)"

	$SED -n "s#^$expr ${soname//./\\.} \(.*\)#\2#p" | sort -u | head -n 1
}

# $1 = ELF file
# $2 = ld cache to use
# $3 = RPATH cache to use (if any)
# $4 = root directory containing libraries
# $5... = packages
get_direct_deps()
{
	local elf="$1"
	local ldcache="$2"
	local rpath_ldcache="$3"
	local binroot="$4"
	shift 4

	local dep
	while read dep
	do
		local target=
		if [ -n "$rpath_ldcache" ]
		then
			target=$(map_soname "$dep" "$@" < "$rpath_ldcache")
		fi
		if [ -z "$target" ]
		then
			target=$(map_soname "$dep" "$@" < "$ldcache")
		fi
		if [ -n "$target" ]
		then
			echo "+$target"
		else
			echo "-$dep"
		fi
	done < <(readelf_filter "$elf" "NEEDED")
}

# $1 = empty if root file, else the parent's name
# $2 = ELF file
# $3 = 1 if dependencies are to be resolved recursively, 0 otherwise
# $4 = ld cache to use
# $5 = RPATH cache to use (if any)
# $6 = root directory containing libraries
# $7... = packages to consider
get_deps()
{
	local root="$1"
	local elf="$2"
	local rec="$3"
	local ldcache="$4"
	local rpath_ldcache="$5"
	local binroot="$6"
	shift 6

	if [ -z "$root" ]
	then
		rpath_ldcache=
		local rpath=$(readelf_filter "$elf" "RPATH")
		local add=0
		if [ -n "$rpath" ]
		then
			add=1
			local dir
			for dir in $libdirs
			do
				if [ "$rpath" = "/$dir" ]
				then
					add=0
					break
				fi
			done
		fi
		if [ $add -ne 0 ]
		then
			rpath_ldcache=$(mktemp)
			read_sonames "$binroot" "${rpath#/}" "$@" > "$rpath_ldcache"
		fi
	fi

	local dep
	for dep in $(get_direct_deps "$elf" "$ldcache" "$rpath_ldcache" "$binroot" "$@")
	do
		echo "$deps" | grep -qF -- "$dep" && continue
		deps="$deps
$dep"
		[ "$rec" -eq 0 ] && continue
		case "$dep" in
		+*)
			get_deps "$elf" "${dep/+}" "$rec" "$ldcache" "$rpath_ldcache" "$binroot" "$@"
			;;
		esac
	done

	[ -z "$root" -a -n "$rpath_ldcache" ] && rm -f "$rpath_ldcache"
}

# $1 = path to ld cache
# $2 = root directory containing libraries
# $3... = packages to consider
create_ld_cache() {
	local ldcache="$1"
	local binroot="$2"
	shift 2

	local dir
	for dir in $libdirs
	do
		read_sonames "$binroot" "$dir" "$@"
	done > "$ldcache"
}

# $1 = path to ld cache
destroy_ld_cache()
{
	rm -f "$1"
}

# $1 = path to ld cache
# $2 = ELF file
# $3... = packages to consider
fill_rpath_cache()
{
	local ldcache="$1"
	local elf="$2"
	local binroot="$3"
	shift 3

	local rpath=$(readelf_filter "$elf" "RPATH")
	if [ -n "$rpath" ]
	then
		read_sonames "$binroot" "${rpath#/}" "$@" >> "$ldcache"
	fi
}

usage()
{
	echo "usage:" >&2
	echo "  $(basename $0) [--recursive] [--use-ld-cache <path-to-cache>] [--] <ELF-file> <bin-root> <package>+" >&2
	echo "  $(basename $0) --create-ld-cache <path-to-cache> <bin-root> <package>+" >&2
	echo "  $(basename $0) --destroy-ld-cache <path-to-cache>" >&2
	echo "  $(basename $0) --fill-rpath-cache <path-to-cache> <ELF-file> <bin-root> <package>+" >&2
	exit 1
}

op=resolve
recursive=0
ldcache=

while [ "${1#--}" != "$1" ]
do
	opt="${1#--}"
	shift
	[ -n "$opt" ] || break
	case "$opt" in
	recursive)
		recursive=1
		;;
	use-ld-cache)
		ldcache="$1"
		shift
		;;
	create-ld-cache)
		op=ldcache_create
		break
		;;
	destroy-ld-cache)
		op=ldcache_destroy
		break
		;;
	fill-rpath-cache)
		op=fill_rpath_cache
		break
		;;
	*)
		usage
		exit 1
		;;
	esac
done

case $op in
resolve)
	[ $# -gt 1 ] || usage
	elffile="$1"
	binroot="$2"
	shift 2
	pwd=$(pwd -P)
	deps=

	destroy=0
	if [ ! -n "$ldcache" ]
	then
		ldcache="$(mktemp)"
		create_ld_cache "$ldcache" "$binroot" "$@"
		destroy=1
	fi
	get_deps "" "$elffile" "$recursive" "$ldcache" "" "$binroot" "$@"
	[ $destroy -eq 1 ] && rm -f "$ldcache"

	echo "$deps" | $SED '1d' | sort
	echo "$deps" | $SED '1d' | grep -q "^-" && exit 2 || exit 0
	;;
fill_rpath_cache)
	[ $# -gt 2 ] || usage
	[ -n "$1" ] || usage
	[ -n "$2" ] || usage
	[ -n "$3" ] || usage
	fill_rpath_cache "$@"
	;;
ldcache_create)
	[ $# -gt 1 ] || usage
	[ -n "$1" ] || usage
	[ -n "$2" ] || usage
	create_ld_cache "$@"
	;;
ldcache_destroy)
	[ $# -eq 1 ] || usage
	[ -n "$1" ] || usage
	destroy_ld_cache "$1"
	;;
esac
