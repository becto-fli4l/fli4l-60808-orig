#!/bin/bash

download()
{
	[ -f $DL/$2 ] && return 0
	wget -O $DL/$2 "$1/$2"
}

extract()
{
	echo "Extracting $2..." >&2
	mkdir -p $BUILD/$1/src
	tar xf $2 -C $BUILD/$1/src --strip-components=1
}

[ -n "$2" ] && BUILD=$2 || BUILD=~/.fli4l-cross/build-$TARGET
[ -n "$3" ] && DL=$3 || DL=$BUILD/../dl

if [ $# -lt 1 -o $# -gt 3 ]; then
	echo "usage: $(basename $0) <prefix> [<build-dir> [<dl-dir>]]" >&2
	echo "" >&2
	echo "After a successful build, there will be two directories:" >&2
	echo "" >&2
	echo "  <build-dir> - contains temporary build files, may be deleted" >&2
	echo "  <dl-dir>    - contains downloaded source archives" >&2
	echo "  <prefix>/$TARGET - contains the cross-compiling environment">&2
	echo "" >&2
	echo "<build-dir> defaults to $BUILD." >&2
	echo "<dl-dir> defaults to $DL." >&2
	echo "There is no default <prefix>. A typical <prefix> is /opt or /usr/local." >&2
	echo "Note that you need write access to this directory." >&2
	echo "" >&2
	echo "Either append <prefix>/$TARGET/usr/bin to your PATH" >&2
	echo "or link all binaries therein into /usr/bin in order to enable" >&2
	echo "the cross-compiling environment!" >&2
	exit 1
fi

SYSROOT=$1/$TARGET
PREFIX=$SYSROOT/usr

rm -rf $BUILD
mkdir -p $BUILD $DL
mkdir -p $PREFIX
export PATH=$PATH:$PREFIX/bin

for p in $PACKAGES; do
	eval run_${p}_pass1 || exit $?
done

for p in $PACKAGES; do
	eval run_${p}_pass2 || exit $?
done

exit 0
