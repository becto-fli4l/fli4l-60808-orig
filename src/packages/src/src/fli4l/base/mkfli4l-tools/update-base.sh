#!/bin/bash
#------------------------------------------------------------------------------
# update-base.sh - updates BASE packages with current sources and binaries
#
# Last Update:  $Id$
#------------------------------------------------------------------------------

BASEDIR=$(dirname $(readlink -f "$0"))/../../../../../base
if [ ! -f "$BASEDIR/version.txt" ]; then
	echo "$(basename $0): Error: Cannot find directory of BASE package!" >&2
	exit 1
fi

COMP=bz2
COMPOPT=j

if [ "$1" = "--force" ]
then
	shift
elif svn info >/dev/null 2>&1; then
	svn update
	if [ -n "$(svn status)" ]; then
		echo "$(basename $0): Sources not clean, aborting!" >&2
		exit 2
	fi
fi

if svn info >/dev/null 2>&1; then
	export VERSION=$(LC_ALL=C svn info | grep "^Last Changed Rev:" | cut -d ' ' -f 4)
else
	export VERSION=
fi
echo "$VERSION" > "$BASEDIR/src/VERSION"

OPTS=
while true
do
	case "$1" in
	-j*)
		OPTS="$OPTS $1"
		shift
		;;
	*)
		break
		;;
	esac
done

set -e
./mkmkfli4l.sh --clean --distro $OPTS "$BASEDIR" "$@"
./mkmkfli4l.sh --build --distro --static $OPTS "$BASEDIR" "$@"
./mkmkfli4l.sh --clean --distro $OPTS "$BASEDIR" "$@"
DEBUG=1 ./mkmkfli4l.sh --clean --distro $OPTS "$BASEDIR" "$@"

echo "$(basename $0): Creating archive..." >&2
tar c${COMPOPT}f "$BASEDIR/src/fli4l-tools.tar.$COMP" \
	--owner=root --group=root --exclude=.svn --exclude=*.l --exclude=*.y *
