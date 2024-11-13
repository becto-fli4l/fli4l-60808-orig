#!/bin/bash
#------------------------------------------------------------------------------
# mkmkfli4l.sh - creates various flavours of mkfli4l and related tools
#
# Last Update:  $Id$
#------------------------------------------------------------------------------

LOG=build.log

printvars()
{
	echo "CHOST=$CHOST CTARGET=$CTARGET OS=$OS ARCH=$ARCH TUNE=$TUNE PREFIX=$PREFIX CROSS=$CROSS STATIC=$STATIC DEBUG=$DEBUG COVERAGE=$COVERAGE"
}

callmake()
{
	( ( PATH=$root/usr/bin:$PATH $M $(printvars) "$@" ) 2>&1 1>&3 | tee >(cat >&4) ) 4>&1 1>>$LOG 3>&1
}

usage()
{
	echo "$(basename $0): Usage:" >&2
	echo "" >&2
	echo "$(basename $0) --build [--static|--distro|--check]* [--] <dest-dir> <path-to-root-of-cross-compiler-toolchain>*" >&2
	echo "  => Builds mkfli4l and related tools. Meaning of options:" >&2
	echo "     --static                                  build a statically linked binary" >&2
	echo "     --distro                                  build executables for BASE package" >&2
	echo "     dest-dir                                  root of destination directory where to install into" >&2
	echo "     path-to-root-of-cross-compiler-toolchain  contains cross-compiler in usr/bin subdirectory" >&2
	echo "" >&2
	echo "$(basename $0) --clean [--distro] [--] <path-to-root-of-cross-compiler-toolchain>*" >&2
	echo "  => Performs a cleanup of the source directory. Meaning of options:" >&2
	echo "     --distro                                  delete executables for BASE package" >&2
	echo "     path-to-root-of-cross-compiler-toolchain  contains cross-compiler in usr/bin subdirectory" >&2
	echo "$(basename $0) --distclean [--distro] [--] <path-to-root-of-cross-compiler-toolchain>*" >&2
	echo "  => Performs a cleanup of the source directory including generated sources. Meaning of options:" >&2
	echo "     --distro                                  delete executables for BASE package" >&2
	echo "     path-to-root-of-cross-compiler-toolchain  contains cross-compiler in usr/bin subdirectory" >&2
	echo "$(basename $0) --docs" >&2
	echo "  => Creates documentation for mkfli4l." >&2
	exit 1
}

do_build()
{
	# determine correct output directory
	case $OS in
	linux|darwin*)
		D=$DESTDIR/unix
		;;
	mingw32)
		D=$DESTDIR/windows
		;;
	*)
		D=$DESTDIR/$OS
		;;
	esac

	echo "$(basename $0): Building with VERSION=$VERSION $(printvars)"
	mkdir -p "$D"

	if [ -n "$CLEANUP" ] && ! callmake distclean
	then
		echo "$(basename $0): Error: Cleanup failed!" >&2
		return 2
	fi

	if ! callmake DESTDIR=$D install
	then
		echo "$(basename $0): Error: Build failed!" >&2
		return 2
	fi
}

do_clean()
{
	echo "$(basename $0): Cleanup with VERSION=$VERSION $(printvars)" >&2
	if callmake clean
	then
		return 0
	else
		echo "$(basename $0): Error: Cleanup failed!" >&2
		return 2
	fi
}

do_distclean()
{
	echo "$(basename $0): Cleanup with VERSION=$VERSION $(printvars)" >&2
	if callmake distclean
	then
		return 0
	else
		echo "$(basename $0): Error: Cleanup failed!" >&2
		return 2
	fi
}

do_check()
{
	echo "$(basename $0): Running tests with VERSION=$VERSION $(printvars)" >&2
	if callmake check
	then
		return 0
	else
		echo "$(basename $0): Error: Cleanup failed!" >&2
		return 2
	fi
}

do_docs()
{
	echo "$(basename $0): Creating documentation for VERSION=$VERSION $(printvars)" >&2
	if callmake docs
	then
		return 0
	else
		echo "$(basename $0): Error: Creating documentation failed!" >&2
		return 2
	fi
}

run()
{
	ACTION="$1"
	shift

	if [ -n "$JOBS" ]
	then
		M="make -j$JOBS"
	else
		M="make"
	fi

	# guessed CHOST which is a canonical system name consisting of
	# ${CPU_TYPE}-${MANUFACTURER}-${KERNEL}-${OPERATING_SYSTEM}
	CHOST=$(sh ./config.guess 2>/dev/null)

	# CTARGETs already used
	CTARGETS=

	for root in "$@"
	do
		if [ -n "$root" ]
		then
			gccs=$(ls $root/usr/bin/*-gcc 2>/dev/null)
		else
			gccs=$(ls /usr/bin/gcc 2>/dev/null)
		fi
		for gcc in $gccs
		do
			[ -x "$gcc" ] || continue
			CROSS=$(basename $gcc | sed -e 's/^\(.*\)gcc$/\1/')

			if [ -n "$CROSS" ]
			then
				CTARGET=$(echo $CROSS | sed -e 's/^\(.*\)-/\1/')
			else
				CTARGET="$CHOST"
			fi

			CTARGET=$(sh ./config.sub $CTARGET 2>/dev/null)
			MACHINE=$(echo $CTARGET | cut -d - -f 1)
			OS=$(echo $CTARGET | cut -d - -f 3 | sed -e 's/^\([a-zA-Z0-9_]*\).*$/\1/')

			# reduce MACHINE for some architectures
			if [ -n "$DISTRO" ]
			then
				case $MACHINE in
				i?86)
					# use minimally i586 machine type and tune for Pentium MMX when building for x86-32 architecture
					MACHINE=i586
					ARCH=$MACHINE
					TUNE=pentium-mmx
					;;
				*)
					ARCH=
					TUNE=
					;;
				esac
			fi

			# stripped CTARGET containing only CPU_TYPE and KERNEL/OPERATING_SYSTEM
			STRIPPED_CTARGET="${MACHINE}-${OS}"

			# check whether CTARGET is already known
			if echo "$CTARGETS" | grep -q "\<$STRIPPED_CTARGET\>"
			then
				continue
			fi

			# check whether other necessary binaries also exist
			LD="$root/usr/bin/${CROSS}ld"
			AR="$root/usr/bin/${CROSS}ar"
			RANLIB="$root/usr/bin/${CROSS}ranlib"
			STRIP="$root/usr/bin/${CROSS}strip"
			if [ ! -x "$LD" ] || [ ! -x "$AR" ] || [ ! -x "$STRIP" ]
			then
				continue
			fi

			# compute prefix for generated binaries
			case $OS in
			mingw32)
				PREFIX=
				;;
			*)
				PREFIX="${STRIPPED_CTARGET}-"
				;;
			esac

			if eval $ACTION
			then
				CTARGETS="$CTARGETS $STRIPPED_CTARGET"
				CLEANUP=1
			else
				return 2
			fi
		done
	done

	[ -n "$CTARGETS" ]
}

[ $# -lt 1 ] && usage
mode="$1"
shift

> $LOG

JOBS=
DISTRO=
CHECK=
ARCH=
TUNE=
CLEANUP=

: ${STATIC:=0}
: ${COVERAGE:=0}
: ${DEBUG:=0}

set -o pipefail

case "$mode" in
--build)
	while true
	do
		case "$1" in
		--static)
			STATIC=1
			shift
			;;
		--distro)
			DISTRO=1
			shift
			;;
		--check)
			CHECK=1
			shift
			;;
		-j*)
			JOBS=$(echo $1 | sed -e 's/^-j//')
			shift
			;;
		--)
			shift
			break
			;;
		*)
			break
			;;
		esac
	done

	[ $# -lt 1 ] && usage
	DESTDIR="$1"
	shift

	if [ $# -gt 0 ]
	then
		run do_build "$@" || exit 2
	else
		run do_build "" || exit 2
	fi
	[ "$CHECK" = "1" ] && run do_check "" || true
	;;

--clean)
	while true
	do
		case "$1" in
		--distro)
			DISTRO=1
			shift
			;;
		-j*)
			JOBS=$(echo $1 | sed -e 's/^-j//')
			shift
			;;
		--)
			shift
			break
			;;
		*)
			break
			;;
		esac
	done

	if [ $# -gt 0 ]
	then
		run do_clean "$@"
	else
		run do_clean ""
	fi
	;;

--distclean)
	while true
	do
		case "$1" in
		--distro)
			DISTRO=1
			shift
			;;
		-j*)
			JOBS=$(echo $1 | sed -e 's/^-j//')
			shift
			;;
		--)
			shift
			break
			;;
		*)
			break
			;;
		esac
	done

	if [ $# -gt 0 ]
	then
		run do_distclean "$@"
	else
		run do_distclean ""
	fi
	;;

--docs)
	run do_docs ""
	;;

*)
	echo "$(basename $0): Error: unknown mode \"$mode\"!" >&2
	usage
	;;
esac
