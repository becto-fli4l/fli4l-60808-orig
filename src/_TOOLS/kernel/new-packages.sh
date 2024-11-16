#!/bin/bash

export LC_ALL=C

############################################################################
# change these whenever a new architecture, net type, or language is added #
############################################################################
ARCHITECTURES="x86 x86_64"
NETTYPES="lan wlan"
LANGUAGES="deutsch english french"

# Generates the documentation's common part. Uses kv_major, kv_a, and kv_b.
# Input:
#   $1 = name of kernel package
generate_common_docs()
{
	mkdir -p "$1/doc/common/tex/$1" || return 1

	cat <<EOF > "$1/doc/common/tex/$1/$1.conf"
KERNEL_VERSION := ${kv_major}

include \$(DOCDIR)/../../../common/tex/doc/kernel.conf

KERNEL_${kv_major}_ADDITIONAL_DEPS := \$(KERNEL_ADDITIONAL_DEPS)
KERNEL_${kv_major}_CREATED_FILES := \$(KERNEL_CREATED_FILES)

ADDITIONAL_DEPS := \$(KERNEL_${kv_major}_ADDITIONAL_DEPS)
CREATED_FILES := \$(KERNEL_${kv_major}_CREATED_FILES)
EOF

	cat <<EOF > "$1/doc/common/tex/$1/$1.tex"
% Last Update: \$Id\$
\def\kernelversion{${kv_a}.${kv_b}}
\def\kernelversionunderscore{${kv_major}}
\def\kernelpkgversionlowercase{${kv_a}\\_${kv_b}}
\def\kernelpkgversionuppercase{${kv_a}\\_${kv_b}}
\input{kernel_common}
EOF

	cat <<EOF > "$1/doc/common/tex/$1/$1_main.tex"
% Last Update: \$Id\$
\def\kernelversion{${kv_a}.${kv_b}}
\def\kernelversionunderscore{${kv_major}}
\def\kernelpkgversionlowercase{${kv_a}\\_${kv_b}}
\def\kernelpkgversionuppercase{${kv_a}\\_${kv_b}}
\input{kernel_common_main}
EOF

	propfiles+=" $1/doc/common/tex/$1/$1.conf $1/doc/common/tex/$1/$1.tex $1/doc/common/tex/$1/$1_main.tex"
}

# Generates the documentation part for a specific language.
# Input:
#   $1 = name of kernel packate
#   $2 = language
generate_language_specific_docs()
{
	mkdir -p "$1/doc/$2/tex/$1" || return 1
	cat <<EOF > "$1/doc/$2/tex/$1/Makefile"
ifeq (\$(DOCDIR),)
	DOCDIR := \$(shell sh -c "if [ -d ../../../../../doc/doc/deutsch/tex/doc ]; then echo '../../../../../doc/doc/deutsch/tex/doc'; elif [ -d ../doc ]; then echo '../doc'; fi")
	ifeq (\$(DOCDIR),)
		DOCDIRS := \$(error "Please install the fli4l documentation package, which contains all necessary files to build ps/pdf/html documentation")
	endif
endif
include \$(DOCDIR)/Make.conf
EOF

	ln -s "../../../common/tex/$1/$1.conf" "$1/doc/$2/tex/$1/$1.conf"
	ln -s "../../../common/tex/$1/$1.tex" "$1/doc/$2/tex/$1/$1.tex"
	ln -s "../../../common/tex/$1/$1_main.tex" "$1/doc/$2/tex/$1/$1_main.tex"

	propfiles+=" $1/doc/$2/tex/$1/Makefile"
}

if [ -z "$1" ]
then
	echo "usage: $(basename $0) <kernel-version>" >&2
	echo "example: $(basename $0) 3.15.6" >&2
	exit 1
fi

year=$(date +%Y)
kv="$1"
kv_major=$(echo "$kv" | sed -n 's/^\([0-9]\+\)\.\([0-9]\+\).*/\1_\2/p')
set -- ${kv_major//_/ }
kv_a=$1
kv_b=$2
package=kernel_${kv_major}
propfiles=

# create directories
mkdir "$package" || exit 2
for dir in changes check config doc opt
do
	mkdir "$package/$dir" || exit 2
done

# create files
cat <<EOF > "$package/changes/$package.txt"
Changelog for package $package
===============================================================================
\$Id$
===============================================================================

EOF

cat <<EOF > "$package/config/$package.txt"
##-----------------------------------------------------------------------------
## fli4l __FLI4LVER__ - configuration for kernel $kv
##
##  P L E A S E  R E A D  T H E  D O C U M E N T A T I O N !
##
##  B I T T E  U N B E D I N G T  D I E  D O K U M E N T A T I O N  L E S E N !
##
##-----------------------------------------------------------------------------
##
## Last Update:  \$Id$
##
## Copyright (c) 2001-$year - Frank Meyer, fli4l-Team <team@fli4l.de>
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##-----------------------------------------------------------------------------
EOF

cat <<EOF > "$package/check/$package.ext"
#------------------------------------------------------------------------------
# fli4l __FLI4LVER__ - extended check script for kernel $kv
#
# Last Update:  \$Id$
#
# Copyright (c) 2001-$year - Frank Meyer, fli4l-Team <team@fli4l.de>
#------------------------------------------------------------------------------

provides kernel_v${kv_a}${kv_b} version $kv
EOF

cat <<EOF > "$package/check/$package.txt"
##-----------------------------------------------------------------------------
## fli4l __FLI4LVER__ - configuration parameters for kernel $kv
##
## You can edit/change this file with any text editor.
##
##        P L E A S E   R E A D   T H E   D O C U M E N T A T I O N ,
##                      S E E   R E A D M E . T X T
##
##        B I T T E  U N B E D I N G T   D I E   D O K U M E N T A T I O N
##              L E S E N ,  S I E H E   R E A D M E . T X T
##
## Last Update:  \$Id$
##
## Copyright (c) 2001-$year - Frank Meyer, fli4l-Team <team@fli4l.de>
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##-----------------------------------------------------------------------------

COMP_TYPE_ROOTFS_V${kv_a}${kv_b}  -  -  RE:gzip|bzip2|lzma|xz 'xz'
EOF

generate_common_docs $package || exit 2
for l in $LANGUAGES
do
	generate_language_specific_docs $package $l || exit 2
done

> "$package/files.txt"
find "$package/" -type f -o -type l | sort |
	sed "s/^$package\/files.txt$/i files.txt/;s/^$package\//d /" >> "$package/files.txt"

svn add "$package"

propfiles+=" $package/config/$package.txt $package/check/$package.txt $package/check/$package.ext $package/changes/$package.txt"

svn propset svn:eol-style "native" $propfiles
svn propset svn:keywords "Author Date Id Revision" $propfiles

echo "########################################################"
echo "PLEASE ADAPT base/check/base.ext ACCORDINGLY!"
echo "########################################################"
