#!/bin/sh
# -------------------------------------------------------------------------
# searches for a precompiled binary                            __FLI4LVER__
#
# Last Update:    $Id$
# -------------------------------------------------------------------------

# $1	 = tool name (e.g. mkfli4l)
# result = relative path to tool (e.g. unix/i486-linux-mkfli4l)
lookup_tool()
{
	name="$1"
	for chost in $chosts_to_try
	do
		oldifs="$IFS"; IFS=-; set -- $chost; IFS="$oldifs"
		machine="$1"
		os="$2"

		case $os in
		linux|darwin*)  dir=unix; toolname="$chost-$name" ;;
		mingw32|cygwin) dir=windows; toolname="$name" ;;
		*)              dir=$os; toolname="$chost-$name" ;;
		esac

		tool="${dir}/${toolname}"
		[ -n "$verbose" ] && echo -n "trying ${tool}..." >&2
		if [ -x "${tool}" ]; then
			[ -n "$verbose" ] && echo " OK." >&2
			echo "$tool"
			return 0
		else
			[ -n "$verbose" ] && echo " not found!" >&2
		fi
	done

	echo "false"
	return 1
}

CHOST=$(sh ./unix/scripts/config.guess 2>/dev/null)
MACHINE=$(echo $CHOST | sed -e 's/^\([^-]*\).*/\1/')
OS=$(echo $CHOST | sed -e 's/^[^-]*-[^-]*-\([^-]*\).*/\1/' | sed -e 's/^\([a-zA-Z0-9_]*\).*$/\1/')
STRIPPED_CHOST="${MACHINE}-${OS}"

chosts_to_try=
case $STRIPPED_CHOST in
i?86-*)
	abi=$(echo $STRIPPED_CHOST | sed -e 's/^.\(.\).*/\1/')
	suffix=$(echo $STRIPPED_CHOST | sed -e 's/^i.86//')
	for i in $(seq $abi -1 3); do
		chosts_to_try="${chosts_to_try} i${i}86${suffix}"
	done
	;;
x86_64-darwin*)
	osx=$(echo $STRIPPED_CHOST | sed -e 's/^x86_64-darwin//')
	for i in $(seq $osx -1 9); do
		chosts_to_try="${chosts_to_try} x86_64-darwin${i}"
	done
	;;
*)
	chosts_to_try="$STRIPPED_CHOST"
	;;
esac
