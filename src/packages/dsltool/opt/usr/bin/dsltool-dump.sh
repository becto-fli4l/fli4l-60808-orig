#!/bin/sh

. /usr/bin/dsltool-helper.sh

# all files in /tmp/dsltool-dump
DATE=`date +"%Y%m%d-%H%M%S"`
DIR=/tmp/dsltool-$DATE-dump/
mkdir $DIR
cd $DIR

# extract dsltool.txt from rc.cfg 
grep DSLTOOL /etc/rc.cfg | sed -e "s/\(DSLTOOL_1_PASS='\).*\('\)/\1pass\2/g" > dsltool.txt

# run dsltool
dsltool $_ARGS -f$DIR -l+ -vvv -o0 -ddsltool-status.pcap -xstatus
dsltool $_ARGS -f$DIR -l+ -vvv -o9 -ddsltool-info.pcap -xinfo

# pack files 
cd ..
tar -czf dsltool-dump.tgz $DIR
rm -rf $DIR
