#!/bin/bash
# $1 = architecture
# $2 = (optional) the name of the package for which to compute the binrev

arch=${1:-x86}
pkg=${2:-*}

# $1 = path
get_last_rev()
{
	svn log --xml "$1" 2>/dev/null |
		sed -n 's/^[[:space:]]*revision="\([0-9]\+\)">$/\1/p' |
		head -n 1
}

svnroot="$(readlink -f "$(dirname "$(readlink -f "$0")")/../../..")"
srcroot="$svnroot/src"
lnkroot="$svnroot/link/$arch"
rooturl="$(svn info "$svnroot" | sed -n 's/^URL:[[:space:]]\+//p')"
binurl="$rooturl/bin/$arch"

# cleanup checkout
svn revert -R "$svnroot"
if [ -d "$lnkroot" ]
then
	svn resolve --accept working -R "$lnkroot"
	rm -f $(svn status "$lnkroot" 2>/dev/null | sed -n 's/^?//p')
fi

# perform checkout
# determine revision of last commit in package directory
srcrootrev=$(svn info --xml --depth=empty "$srcroot" |
		sed '1,/^<entry$/d' |
		sed '/^<url>/,$d' |
		sed -n 's/.*revision="\([^"]\+\)".*/\1/p' 2>/dev/null)

svn update -r $srcrootrev --set-depth empty "$svnroot/link"
svn update -r $srcrootrev --set-depth infinity "$lnkroot"
svn update -r $srcrootrev --set-depth immediates "$srcroot/packages"
svn update -r $srcrootrev --set-depth files "$srcroot/packages"/$pkg

filestocommit=
for filestxt in "$srcroot/packages/"$pkg/files.txt
do
	package=$(basename "$(dirname "$filestxt")")
	if [ ! -f "$lnkroot/$package.rev" -o "$package" = "$pkg" ]
	then
		echo -n "computing binary revision for $package..." >&2
		binrev=0
		for f in $(sed -n "s/^[bB] \($arch::\)\?opt\/\(.*\)$/\2/p" "$filestxt")
		do
			echo -n "." >&2
			rev=$(get_last_rev "$binurl/$f@$srcrootrev")
			[ -n "$rev" ] || continue
			[ $rev -gt $binrev ] && binrev=$rev
		done
		echo >&2
		if [ $binrev -ne 0 ]
		then
			echo "$binrev" > "$lnkroot/$package.rev"
			filestocommit+=" $lnkroot/$package.rev"
		fi
	fi
done

if [ -n "$filestocommit" ]
then
	svn add --force $filestocommit
	if ! svn ci $SVNOPTIONS -m "FFL-870: package binary links created or updated" "$lnkroot"
	then
		echo "error: FFL-870 commit failed" >&2
		exit 2
	fi
fi

exit 0
