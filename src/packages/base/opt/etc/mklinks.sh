#! /bin/sh
#----------------------------------------------------------------------------
# mklinks.sh
#
# Last Update:  $Id$
#
# usage: /pathname/mklinks.sh source-tree dest-tree
#
# example: /etc/mklinks.sh /opt/usr /usr
#
# start always with absolute pathname!
#----------------------------------------------------------------------------

pgm=$0
source=$1
dest=$2

cd $source

for j in *
do
    if [ -d $j ]
    then
        mkdir -p $dest/$j
        sh $pgm $source/$j $dest/$j
    elif [ ! -f $dest/$j ]
    then
        ln -s $source/$j $dest/$j
    else
        colecho "$pgm: skipping $dest/$j, file already exists" br x br 1>&2
        echo "$pgm: skipping $dest/$j, file already exists" >> /bootmsg.txt
    fi
done
