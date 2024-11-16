#!/bin/bash
#------------------------------------------------------------------------------
# mktarball.sh - copy a package, then create tarball
#
# usage: mktarball.sh package target-directory version
#
# examples:
#
#   sh mktarball.sh base /tmp 4.0.0
#
#      - copy files to /tmp/base
#      - build documentation
#      - rename /tmp/base to /tmp/fli4l-<version>
#      - create tarball /tmp/fli4l-<version>.tar.gz
#
#   sh mktarball.sh dhcp /tmp 4.0.0
#
#      - copy files to /tmp/dhcp
#      - build documentation
#      - create tarball /tmp/dhcp.tar.gz
#
#
# Creation:	17.09.2001  fm
# Last Update:  $Id$
#------------------------------------------------------------------------------

package=$1
targetdir=$2
version=$3

if [ -z "$package" -o -z "$targetdir" -o -z "$version" ]
then
	echo "usage: $(basename $0) <package> <temp-dir> <version>" >&2
	exit 1
fi

curdir="$(dirname "$(readlink -f "$0")")"
cd $targetdir || exit 2
$curdir/_TOOLS/jenkins/mkarchive.sh --use-checkout --version=$version $package ${ARCH:-x86} || exit 3

# special handling... we should get rid of it eventually!
case $package
in
base)
	pkgname=fli4l-$version
	mkdir $pkgname/
	tar -C $pkgname/ -xzf base-*.tar.gz || exit 4
	tar -czf $pkgname.tar.gz --owner=root --group=root $pkgname/ || exit 5
	rm -rf $pkgname/ base-$version.tar.gz
	;;

doc)
	pkgname=fli4l-$version
	mkdir doc/
	tar -C doc/ -xzf doc-$version.tar.gz || exit 4
	for x in doc/doc/*/pdf/doc.pdf
	do
		[ -f $x ] && mv $x $(dirname $x)/$pkgname.pdf
	done
	for x in doc/doc/*/html/doc
	do
		[ -d $x ] && mv $x $(dirname $x)/$pkgname
	done
	(cd doc && tar -czf ../$pkgname-doc.tar.gz --owner=root --group=root .) || exit 5
	rm -rf doc/ doc-$version.tar.gz
	;;

*)
	mv $package-$version.tar.gz $package.tar.gz
	;;
esac

exit 0
