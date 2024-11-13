#!/bin/sh
##-----------------------------------------------------------------------------
## _mkfli4lsvn.sh - make an fli4l installation based on an SVN checkout
##
## You can edit/change this file with any text editor.
##
## Creation    :  2006 lanspezi
## Last Update :  $Id$
##
## Copyright (c) 2006-2016 - Peter Schiefer <peter@fli4l.de>
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##-----------------------------------------------------------------------------


# default values for some variables
# ---------------------------------
#(edit or set variable in external file _mkfli4lsvn.conf)

packages="base dns_dhcp dhcp_client dsl dyndns easycron hd httpd isdn openvpn sshd tools wlan kernel_3_2 rrdtool3 hwsupp"
outdir=~/"fli4l_inst/fli4l-svn"
log=~/"fli4l_inst/log.txt"
#mybin_sourcedir=~/"fli4l_config/mybin"
remove_configdir="no"
verbose="yes"
arch="x86"


# no changes needed after this line
# ---------------------------------

## for later use of $SED (MacOS vs. Linux sed)
type -p gsed >/dev/null && SED=gsed || SED=sed

myecho ()
{
    if [ "$verbose" = "yes" ]
    then
        echo "$1" "$2"
    fi
}

# set variables with own configuration file
# -----------------------------------------
if [ -f "$1" ]
then
    . "$1"
elif [ -f _mkfli4lsvn.conf ]
then
    . ./_mkfli4lsvn.conf
fi

# some checks
# -----------
if [ ! -d ${outdir} ]
then
    echo "Error: ${outdir} does not exist ..."
    echo "create it (mkdir ${outdir}) and start again"
    exit 1
fi
if [ ! -f packages/base/version.txt ]
then
    echo "Error: current working directory is not the base directory of a checked out repository..."
    exit 1
fi
if [ -d ../.svn ]
then
	SVN=svn
elif [ -f ../.git/svn2git ]
then
	echo "git-svnimport repository not supported!"
	exit 1
elif [ -d ../.git/svn ]
then
	SVN=git-svn
elif svk info > /dev/null 2>&1
then
	SVN=svk
elif [ -d ../.hg/svn ]
then
	SVN=hgsvn
elif [ -d ../.hg ]
then
	SVN=hg
elif [ -z "$SVN" ]
then
	echo "Don't know what kind of repository you use!"
	exit 1
fi

# here starts the real fun
echo
echo "Updating fli4l (${outdir})"
echo "-------------------------------------------------------------------------------"
echo "removing old files..."

remove="changes check doc img logo opt src unix windows"

if [ -f ${log} ]
then
    myecho "    removing ${log}"
    rm ${log}
fi

if [ "$remove_configdir" = "yes" ]
then
    if [ -d ${outdir}/config ]
    then
        myecho "    removing ${outdir}/config/"
        rm -rf ${outdir}/config
    fi
fi

for i in $remove
do
    if [ -d ${outdir}/$i ]
    then
        myecho "    removing ${outdir}/$i/"
        rm -rf ${outdir}/$i
    fi
done

# remove all files in the top directory of the fli4l hierarchy
for i in `$SED -e '/^[iI]/d;s/^. //;/.*\/.*/d' packages/*/files.txt`
do
    myecho "    removing ${outdir}/$i"
    rm -f ${outdir}/$i
done

case $SVN in
	svn|svk)
		rev=$(svnversion -c ..)
		rev=${rev##*:}
		branch=$(LC_ALL=C svn info | grep "^URL:" | $SED -e 's,^.*/\([^/]\+\)/src$,\1,')
		[ "$branch" = "trunk" ] && branch= || branch="-$branch"
	;;
	git-svn)
		n=0
		while [ -z "$rev" ]
		do
			rev=`git svn find-rev HEAD~$n`
			n=$(($n + 1))
		done
		[ -n "$(git status --porcelain)" ] && rev="${rev}M"
		branch=$(git branch | sed -n 's/^* //p')
		[ "$branch" = "master" ] && branch= || branch="-$branch"
	;;
	hgsvn)
		rev=$(LC_ALL=C hg svn info | sed -n 's/^Last Changed Rev: //p')
		[ -n "$(hg status)" ] && rev="${rev}M"
		set -- $(hg id -bB)
		branch=$1
		bookmark=$2
		if [ -n "$bookmark" -a "$bookmark" != "@" ]
		then
			branch="-$branch@$bookmark"
		else
			[ "$branch" = "default" ] && branch= || branch="-$branch"
		fi
	;;
	hg)
		rev=$(LC_ALL=C hg id -n)
		set -- $(hg id -bB)
		branch=$1
		bookmark=$2
		if [ -n "$bookmark" -a "$bookmark" != "@" ]
		then
			branch="-$branch@$bookmark"
		else
			[ "$branch" = "default" ] && branch= || branch="-$branch"
		fi
	;;
esac
if [ -z "$rev" ]
then
	echo "Error finding current revision!"
	exit 1
fi

ver=`cat packages/base/version.txt`-r$rev$branch
ARCH=$arch; export ARCH

echo "updating fli4l files to $ver"

for i in $packages
do
    if [ -d packages/$i ]
    then
        myecho -n "    copying $i... "
        if ! ./mktarball packages/$i ${outdir} $ver >> ${log}
        then
            myecho "Please check repository with check-files.pl"
            exit 1
        else
            myecho "finished"
        fi
    else
        myecho "    skipping package $i because no source directory for $i exists"
    fi
done
echo $ver > ${outdir}/version.txt
echo $arch > ${outdir}/arch.txt

#if [ "${mybin_sourcedir}" -a -d "${mybin_sourcedir}" ]
#then
#    cp ${mybin_sourcedir}/* ${outdir}/opt/usr/local/mybin/
#fi

echo "update finished"

