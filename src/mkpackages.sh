#!/bin/sh
#------------------------------------------------------------------------------
# mkpackages.sh - create tarballs from multiple packages
#
# usage: mkpackages.sh config-file
#
# the config-file ist just a shell script which includes 5 variables:
#
#   packages - a list of all packages to be created. "doc" must be the last!
#   fli4ldir - directory of a fresh svn checkout of fli4l
#   outdir   - directory where the tarballs are placed in the end
#   nightly  - "yes" to build  a version with "svn<date>" added to the
#              version number
#   svnup    - "yes" to do a svn update before building the packages
#   console  - "yes" to output messages to console and log.txt
#
#
# Creation:     in 2002 tobig
# Last Update:  $Id$
#------------------------------------------------------------------------------

if [ "$#" != 1 -o ! -f "$1" ]
then
    echo "usage: `basename $0` config-file" >&2
    exit 1
fi

# Read config
. $1

tmpdir="/tmp/tarball.$$"
export TEXFD=3

mkdir -p "$tmpdir"
mkdir -p "$outdir"

cd $fli4ldir

if [ "$svnup" = "yes" ]
then
    # Get SVN Update
    svn up -q
fi

# Check if version file is available
if [ ! -f packages/base/version.txt ]
then
    echo "Missing packages/base/version.txt, please update repository."
    exit 1
fi

if [ "$console" = "yes" ]
then
    exec 2>&1 3> $outdir/log_tex.txt 4>$outdir/log_verb.txt
else
    exec > $outdir/log.txt 2>&1 3> $outdir/log_tex.txt 4>$outdir/log_verb.txt
fi

# Modify version to state nightly build
version=`cat packages/base/version.txt`
if [ "$nightly" = "yes" ]
then
    rev=$(svnversion -c ..)
    rev=${rev##*:}
    branch=$(LC_ALL=C svn info | grep "^URL:" | sed -e 's,^.*/\([^/]\+\)/src$,\1,')
    [ "$branch" = "trunk" ] && branch= || branch="-$branch"
    version=$version-r$rev$branch
fi

echo "Building fli4l Version $version $ARCH [`date +%H:%M:%S`]"

failed=0
for pack in $packages
do
    if [ -d "packages/$pack" ]
    then
        echo -n "Creating $pack... "
        echo "Creating $pack... " 1>&4
        if sh ./mktarball.sh $pack $tmpdir $version 1>&4
        then
            echo "ok [`date +%H:%M:%S`]"
        else
            echo "failed [`date +%H:%M:%S`]"
            failed=1
        fi
    else
        echo "Directory 'packages/$pack' does not exist, skipping!"
    fi
done

rm -f $outdir/*.tar.gz
cp $tmpdir/*.tar.gz  $outdir/
rm -Rf $tmpdir

# search for variables or files to translate and put them into a html file
transneed=

allpath='packages/$pack/opt/srv/www/lang/* packages/$pack/opt/srv/www/c3surf/lang packages/$pack/opt/usr/lib/hdinstall/* packages/$pack/opt/usr/lib/c3surf packages/$pack/windows/imonc/*.lan packages/$pack/windows/scripts/_msg_*'

for pack in $packages
do
    totalpath=$allpath
    # asterisk in, asterisk out needed to handle multiple match /doc
    for lang in `grep "doc/.*/tex/" "packages/$pack/files.txt" | sed -e "s/^..//;s/\(\/tex\/.*\/\).*/\1*.tex/;s/\/tex\//\/tex\/*/"';s/^.*\/doc/$pack\/doc/'";s/\/tex\/\*/\/tex\//" | sort -u`
    do
        totalpath=$totalpath" "packages/$pack/$lang
    done

    set -f
    for langpath in $totalpath
    do
        eval langpath=$langpath
        set +f
        if grep -q "##TRANSLATE##" $langpath 2>/dev/null
        then
            set -f
            path=`echo $langpath | sed "s/packages\/$pack//;s/*//g;s/\.lan//;s/_msg_//;s/\.tex//"`
            set +f
            grep -n "##TRANSLATE##" $langpath |
            sed -e "s°packages/$pack$path°$pack:$path:°;s°##TRANSLATE##°°;s/[%;]//" \
                -e "s°<°\&lt\;°g; s°>°\&gt\;°g; s°'°<code><i>°g; s°´°</i></code>°g; " \
                -e "s°#°°g; s°:° </td><td> °g;s°^° <tr><td>°;s°$° </td></tr> °;" >> /tmp/translation.$$
            transneed='yes'
        fi
    done
done
set +f
if [ "$transneed" ]
then
    echo -n "creating translation.html... "
    cat > $outdir/translation.html <<-EOF
	<html><head><title>Variables or Files to translate</title></head>
	<body>
	<center><h1>Variables or Files to translate ($version)</h1></center>
	<table border="1">
	<tr bgcolor="#dddddd"><td>package</td><td> path</td><td> file</td><td>row</td><td>variable</td><td>comment  </td></tr>
EOF
    cat /tmp/translation.$$ >> $outdir/translation.html
    cat >> $outdir/translation.html <<-EOF
	</table></body></html>
EOF
    rm /tmp/translation.$$
    echo "ok [`date +%H:%M:%S`]"
else
    rm -f $outdir/translation.html
fi


echo "Finished! [`date +%H:%M:%S`]"
# use exit code of zero if all archives have been built
# use exit code greater zero if at least one archive could not be built
exit $failed
