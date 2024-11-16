#!/bin/bash

USAGE="usage:
  $(basename $0) <option>* <package> <arch>

where:
  <package> is the package which to create the archive for
  <arch> is the architecture which to create the archive for
  <option> is one of the following options:

--use-checkout: an existing checkout is used instead of setting up a new one
--version=<v>:  use <v> as archive name suffix instead of the computed version
--nohtml:       don't build the HTML documentation

"

# prints an informative message to stdout
message()
{
	echo "$@"
}

# prints a warning message to stderr
warning()
{
	message "warning: $@" >&2
}

# prints an error message to stderr
error()
{
	message "error: $@" >&2
}

# returns the maximum revision for some SVN objects
# $1... = SVN objects (files, directories etc.)
get_max_commit_revision()
{
	local result=0 rev f
	for f
	do
		local rev=$(svn info --xml "$f" 2>/dev/null |
			xmllint --xpath 'string(/info/entry/commit/@revision)' -)
		: ${rev:=0}
		[ $rev -gt $result ] && result=$rev
	done
	echo $result
}

# force consistent sorting and messages
export LC_ALL=C

use_checkout=
version=
nohtml=

set -- $MKARCHIVE_OPTIONS "$@"
while true
do
	case $1 in
	--use-checkout)
		use_checkout=1
		shift
		;;
	--version=*)
		version=${1#--version=}
		shift
		;;
	--nohtml)
		nohtml=1
		shift
		;;
	--*)
		warning "unknown option $1, ignoring"
		shift
		;;
	*)
		break
		;;
	esac
done

package=$1
arch=$2

# for Jenkins compatibility, remove if all Jenkins jobs have been converted
# to the new syntax
if [ "$arch" = "." ]
then
	arch=$3
fi

export ARCH=$arch

if [ -z "$package" -o -z "$arch" ]
then
	echo "$USAGE" >&2
	exit 1
fi

svnroot="$(readlink -f "$(dirname "$(readlink -f "$0")")/../../..")"
srcroot="$svnroot/src"
binroot="$svnroot/bin/$arch"
lnkroot="$svnroot/link/$arch"

rooturl="$(svn info --xml "$svnroot" | xmllint --xpath 'string(/info/entry/url)' -)"
binurl="$rooturl/bin/$arch"
lnkurl="$rooturl/link/$arch"

branch="$(echo "$rooturl" | sed 's,^.*/\([^/]\+\)$,\1,')"
category="$(echo "$rooturl" | sed 's,^.*/\([^/]\+\)/[^/]\+$,\1,')"

if [ -z "$use_checkout" ]
then
	# perform checkout
	# determine revision of package directory
	srcrootrev="$(svn info --xml "$srcroot" | xmllint --xpath 'string(/info/entry/@revision)' -)"
	if [ -z "$srcrootrev" ]
	then
		error "failed to determine source revision"
		exit 2
	fi

	svn update -r $srcrootrev --set-depth empty "$svnroot/bin"
	svn update -r $srcrootrev --set-depth empty "$binroot"
	svn update -r $srcrootrev --set-depth empty "$binroot/packages"
	svn update -r $srcrootrev --set-depth empty "$lnkroot"
	svn update -r $srcrootrev --set-depth empty "$lnkroot/$package.rev"
fi

add_entries=
case $package in
base)
	# base needs files found in kernel packages
	if [ -z "$use_checkout" ]
	then
		svn update -r $srcrootrev --set-depth empty "$srcroot/packages"
		svn update -r $srcrootrev --set-depth infinity "$srcroot/packages/base"
		svn update -r $srcrootrev --set-depth infinity "$srcroot/packages/doc"
		svn update -r $srcrootrev --set-depth infinity "$binroot/packages"
	fi
	add_entries="$binroot"/packages/kernel_*/doc
	;;
doc)
	# doc needs documentation of all other packages and "base/version.txt"
	if [ -z "$use_checkout" ]
	then
		svn update -r $srcrootrev --set-depth infinity "$srcroot/packages"
		svn update -r $srcrootrev --set-depth infinity "$binroot/packages"
	fi
	add_entries="$srcroot"/packages/*/doc\ "$binroot"/packages/*/doc\ "$srcroot"/packages/base/version.txt
	;;
*)
	# all other packages need "doc" and "base/version.txt"
	if [ -z "$use_checkout" ]
	then
		svn update -r $srcrootrev --set-depth empty "$srcroot/packages"
		svn update -r $srcrootrev --set-depth infinity "$srcroot/packages/$package"
		svn update -r $srcrootrev --set-depth infinity "$srcroot/packages/doc"
		svn update -r $srcrootrev --set-depth files "$srcroot/packages/base"
		svn update -r $srcrootrev --set-depth infinity "$binroot/packages/$package"
	fi
	add_entries="$srcroot"/packages/doc\ "$srcroot"/packages/base/version.txt

	case $package in
	kernel_*)
		# kernel packages need the changelog found in the default kernel
		# package
		kdef=$(echo $package | sed 's/^\(kernel_[0-9]\+_[0-9]\+\).*/\1/')
		if [ "$kdef" != "$package" ]
		then
			if [ -z "$use_checkout" ]
			then
				svn update -r $srcrootrev --set-depth infinity "$srcroot/packages/$kdef"
			fi
			add_entries+=\ "$srcroot"/packages/$kdef/changes/$kdef.txt
		fi
		;;
	esac
	;;
esac

if [ ! -f "$srcroot/packages/$package/files.txt" ]
then
	error "'$package' does not seem to be a valid package, exiting"
	exit 3
fi

# determine srcrev as the revision of last commit in the package directories
srcrev=$(get_max_commit_revision "$srcroot"/packages/$package "$binroot"/packages/$package $add_entries)

# determine binrev as read from the link file
if [ -s "$lnkroot/$package.rev" ]
then
	read binrev < $lnkroot/$package.rev
fi
: ${binrev:=0}

# determine pkgrev as the maximum of $srcrev and $binrev; this revision is
# eventually used to get the files from bin/
if [ $srcrev -gt $binrev ]
then
	pkgrev=$srcrev
else
	pkgrev=$binrev
fi

# calculate package version string
if [ -z "$version" ]
then
	base_version_file="$srcroot/packages/base/version.txt"
	if [ -s "$base_version_file" ]
	then
		read base_version < "$base_version_file"
	fi
	: ${base_version:=$category}
	version=$base_version-$branch-$arch-r$pkgrev
fi

# calculate archive name
archive_name="$package-$version"

# do the real work
tgtdir=$(mktemp -d --tmpdir "$(basename "$0").XXXXXXXXXX")
if [ ! -d "$tgtdir" ]
then
    error "cannot create temporary directory, exiting"
    exit 4
else
    trap "rm -rf \"$tgtdir\"" EXIT
fi

message "Starting job for package $package, version $version"

message "collecting files"
if ! "$srcroot/mktarball" "$srcroot/packages/$package" "$tgtdir" "$version" "$binurl" "$pkgrev"
then
	error "mktarball failed, exiting"
	exit 5
fi

# put additional information into archive
message "generating version information"
mkdir -p "$tgtdir/version"
cat <<-EOF | unix2dos > "$tgtdir/version/$package.txt"
	PACKAGE="$package"
	CATEGORY="$category"
	BRANCH="$branch"
	ARCH="$arch"
	REVISION="$pkgrev"
EOF

[ -f "$tgtdir/version.txt" ] && echo "$version" > "$tgtdir/version.txt"
[ -f "$tgtdir/arch.txt" ] && echo "$arch" > "$tgtdir/arch.txt"

# build documentation
ok=1
message "building documentation"

if [ -n "$nohtml" ]
then
	doc_formats=pdf
else
	doc_formats="pdf html"
fi

: ${TEXFD:=1}

for langdir in "$srcroot/packages/$package/doc"/*
do
	lang=$(basename "$langdir")
	[ -d "$langdir/tex" ] || continue
	for path in "$langdir/tex"/*
	do
		pkg=$(basename "$path")
		[ -f "$path/Makefile" ] || continue

		for format in $doc_formats
		do
			type=${format^^*}
			case $format in
			html) suffix=;;
			*)    suffix=".$format";;
			esac

			message "building $type documentation for package $pkg, language $lang"
			if ! make -C "$path" PACKET=$pkg TEXMODE=batchmode \
				DOCDIR="$srcroot/packages/doc/doc/$lang/tex/doc" \
				VERSION="$version" ARCH=$arch $format >&$TEXFD 2>&1
			then
				error "building $type documentation failed for package $pkg, language $lang"
				ok=
			else
				# move documentation
				mkdir -p "$tgtdir/doc/$lang/$format"
				mv "$path/$pkg$suffix" "$tgtdir/doc/$lang/$format/"
			fi

			# cleanup
			make -C "$path" PACKET=$pkg TEXMODE=batchmode \
				DOCDIR="$srcroot/packages/doc/doc/$lang/tex/doc" \
				VERSION="$version" ARCH=$arch distclean >&$TEXFD 2>&1
		done
	done
done

if [ -z "$ok" ]
then
	error "failed to build documentation, exiting"
	exit 6
fi

# build archive
if ! tar -C "$tgtdir" -chzf "$archive_name.tar.gz" --owner=root --group=root .
then
	error "creating archive using 'tar' failed, exiting"
	rm -f "$archive_name.tar.gz"
	exit 7
fi

exit 0
