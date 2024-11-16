#!/bin/bash

# input:
#  $1 = architecture (optional)
#  $2 = if equal to "--force-rebuild", the FBR is completely rebuilt

# force consistent sorting and messages
# note that LC_ALL=C does not work as we have entries with UTF-8 encoded names
# in our repository...
export LC_ALL=en_US.UTF-8
export FBR_ARCH=${1:-x86}

force_rebuild=
[ "$2" = "--force-rebuild" ] && force_rebuild=1

svnroot="$(readlink -f "$(dirname "$(readlink -f "$0")")/../../..")"
binroot="$svnroot/bin/$FBR_ARCH"
srcroot="$svnroot/src"
lnkroot="$svnroot/link/$FBR_ARCH"
branch="$(svn info "$svnroot" | grep "^URL:" | sed 's,^.*/\([^/]\+\)$,\1,')"
category="$(svn info "$svnroot" | grep "^URL:" | sed 's,^.*/\([^/]\+\)/[^/]\+$,\1,')"

echo "Starting FBR job for $category-$branch/$FBR_ARCH"

echo "Cleaning up checkout..."
svn cleanup "$svnroot"
svn revert -R "$svnroot"

binrevfile="$binroot/src.rev"
if [ -s "$binrevfile" ]
then
	read binrev < "$binrevfile"
	echo "Binary revision: $binrev"
else
	binrev=0
	echo "Binary revision not available, assuming nonexisting binaries"
fi

# determine revision of last commit in src/packages/src/src
srcrev=$(svn status -v --xml --depth=empty "$srcroot/packages/src/src" |
		sed '1,/^<commit$/d' |
		sed -n 's/.*revision="\([^"]\+\)".*/\1/p' 2>/dev/null)

if [ -z "$force_rebuild" ] && [ -z "$srcrev" ]
then
	echo "error: cannot determine source revision" >&2
	exit 2
else
	echo "Source revision: $srcrev"
fi

if [ -z "$force_rebuild" ] && [ "$srcrev" -lt "$binrev" ]
then
	echo "error: source revision $srcrev is older than binary revision $binrev" >&2
	exit 3
else
	echo "Performing update of binaries from $binrev to $srcrev..."

	if [ -n "$force_rebuild" ]
	then
		if ! echo | "$srcroot/packages/src/src/fbr/fbr-make" clean
		then
			echo "error: cleaning FBR failed" >&2
			exit 8
		fi
	fi

	if ! "$srcroot/packages/src/src/fbr/fbr-make" world legal-info
	then
		echo "error: building FBR failed" >&2
		exit 4
	fi
fi

# always do post-processing
echo "Updating binaries to r$srcrev..."
if ! "$srcroot/packages/src/src/fbr/fbr-make" update-repo-binaries \
	--delete-missing --update-svn --update-kernel-files \
	--update-legal-info "$svnroot"
then
	echo "error: updating binaries failed" >&2
	exit 5
fi

echo "$srcrev" > "$binrevfile"
svn add --force "$binrevfile"

changed=$(svn status --xml "$binroot" | sed -n "s/^[[:space:]]*path=\"${binroot////\\/}\/\([^\"]\+\)\">\$/\1/p")

if ! svn ci -m "FFL-758: $FBR_ARCH binaries updated, ref. r${srcrev}" "$binroot"
then
	echo "error: FFL-758 commit failed" >&2
	exit 6
fi

revision=$(svnversion "$binroot" | sed -n 's/\([0-9]\+:\)\?\([0-9]\+\)[A-Z]*/\2/p')
echo "Updating package links to r$revision..."

packages=
filestocommit=
add=1

if [ ! -d "$lnkroot" ]
then
	mkdir "$lnkroot"
	filestocommit+=" $lnkroot"
	add=
fi

max=100
while [ -n "$changed" ]
do
	for package in $(head -n $max <<< "$changed" | sed -n 's,^packages/\([^/]\+\)/.*$,\1,p')
	do
		echo "$packages" | grep -q " $package\( \|\$\)" && continue
		packages+=" $package"

		revfile="$lnkroot/$package.rev"
		[ -n "$add" ] && filestocommit+=" $revfile"
		echo "$revision" > "$revfile"
	done

	regex="$(head -n $max <<< "$changed" | sed 's/\\/\\\\/g;s/\./\\./g;s/\*/\\*/g;s/\[/\\[/g;s/\]/\\]/g' | sed ':a;N;$!ba;s/\n/\\|/g')"
	changed="$(sed "1,${max}d" <<< "$changed")"

	for filestxt in $(grep -l "^. \($FBR_ARCH::\)\?\(opt/\(files/\)\?\)\?\($regex\)$" "$srcroot"/packages/*/files.txt "$binroot"/packages/*/files.txt 2>/dev/null)
	do
		package=$(basename $(dirname "$filestxt"))
		echo "$packages" | grep -q " $package\( \|\$\)" && continue
		packages+=" $package"

		revfile="$lnkroot/$package.rev"
		[ -n "$add" ] && filestocommit+=" $revfile"
		echo "$revision" > "$revfile"
	done
done

if [ -n "$filestocommit" ]
then
	svn add --force $filestocommit
	if ! svn ci -m "FFL-870: package files updated due to $FBR_ARCH binary update, ref. r${revision}" "$lnkroot"
	then
		echo "error: FFL-870 commit failed" >&2
		exit 7
	fi
fi

# everything succeeded, we are happy
echo "Success."
exit 0
