#!/bin/bash

PKGROOT="$(readlink -f "$(dirname "$(readlink -f "$0")")/../../..")/src/packages"
: ${ARCH:=x86}
BINROOT="$(readlink -f "$(dirname "$(readlink -f "$0")")/../../..")/bin/$ARCH/packages"

# $1 = kernel_package
# $2 = lan|wlan
check_regex()
{
	local kernel_package=$1 type=$2
	local mainpkg
	case $type in
		lan)  mainpkg=base;;
		wlan) mainpkg=wlan;;
	esac

	while read BUS DRIVER DESCRIPTION
	do
		grep -q "=.*\<$DRIVER\>" $PKGROOT/$mainpkg/check/$mainpkg.exp &&
			continue

		if [ -f $PKGROOT/$BUS/check/$BUS.exp ]
		then
			grep -q "=.*\<$DRIVER\>" $PKGROOT/$BUS/check/$BUS.exp &&
				continue
			echo "-VARTYPE- please add $type driver \"$DRIVER\" [$BUS] to $BUS/check/$BUS.exp for kernel $kernel_package"

		else
			echo "-VARTYPE- please add $type driver \"$DRIVER\" [$BUS] to $mainpkg/check/$mainpkg.exp for kernel $kernel_package"
		fi

		if grep -q "=.*\<$DRIVER\>" $PKGROOT/*/check/*.exp
		then
			echo "------ Driver found in (is this correct?):"
			grep "=.*\<$DRIVER\>" $PKGROOT/*/check/*.exp
			echo "------"
		fi
	done < $BINROOT/$kernel_package/doc/common/tex/$kernel_package/niclist_${kernel_package}_${type}.csv
}

# $1 = kernel_package
# $2 = lan|wlan
check_package()
{
	local kernel_package=$1 type=$2
	local mainpkg
	case $type in
		lan)  mainpkg=base;;
		wlan) mainpkg=wlan;;
	esac

	while read BUS DRIVER DESCRIPTION
	do
		grep -q "\<$DRIVER\.ko\>" $PKGROOT/$mainpkg/opt/${mainpkg}*.txt &&
			continue

		if [ -f $PKGROOT/$BUS/opt/$BUS.txt ]
		then
			grep -q "\<$DRIVER\.ko\>" $PKGROOT/$BUS/opt/${BUS}*.txt &&
				continue
			echo "-PACKAGE- please add $type driver \"$DRIVER\" [$BUS] to $BUS/opt/$BUS.txt for kernel $kernel_package"

		else
			echo "-PACKAGE- please add $type driver \"$DRIVER\" [$BUS] to $mainpkg/opt/$mainpkg.txt for kernel $kernel_package"
		fi

		if grep -q "\<$DRIVER\.ko\>" $PKGROOT/*/opt/*.txt
		then
			echo "------ Driver found in (is this correct?):"
			grep "\<$DRIVER\.ko\>" $PKGROOT/*/opt/*.txt
			echo "------"
		fi
	done < $BINROOT/$kernel_package/doc/common/tex/$kernel_package/niclist_${kernel_package}_${type}.csv
}

old_IFS=$IFS
IFS='#'
for k in $PKGROOT/kernel_*
do
	kernel_package=${k##*/}
	echo "$kernel_package..."
	check_regex $kernel_package lan
	check_regex $kernel_package wlan
	check_package $kernel_package lan
	check_package $kernel_package wlan
done
