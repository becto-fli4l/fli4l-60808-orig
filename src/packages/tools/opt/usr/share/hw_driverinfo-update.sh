#!/bin/sh

# Extracts driver Descriptions from all Kernel-Modules

: ${ARCH:=x86}
FILE='hw_driverinfo.txt'

DIRS=`ls ../../../../ | grep "^kernel_" | sort -r`
for k in $DIRS
do
  KERNEL_VERSION=`sed -n "s/^. $ARCH::opt\/files\/lib\/modules\/\([^/]\+\)\/modules.alias$/\1/p" ../../../../$k/files.txt`
  find ../../../../../../bin/$ARCH/lib/modules/$KERNEL_VERSION/ -name \*.ko
done >/tmp/$$filelist.tmp

sed 's#.*/##' /tmp/$$filelist.tmp | sort -u >/tmp/$$driverlist.tmp

filelist=/tmp/$$filelist.tmp
cat /tmp/$$driverlist.tmp |\
  (
    while read line
    do
      grep "/$line$" $filelist
    done
  ) |\
  (
    while read line
    do
      driver=`basename $line | sed 's/.ko//'`
      info=`/sbin/modinfo $line | sed -n 's/^description://p' | sed 's/^ //g' | sed 's/ [dD]river//' | sed 's/[Ss]upport for //g' | sed 's/^ *\(SCSI \|ATA \|A \|\)low-level for //g' | sed 's/ \(low-level\|\(controller\|sound\(card\|\)\)\(s\|\)\)$//g' | sed 's/^the //g' | sed 's/\\$//g' | sed 's/^ *for //' | sed 's/PCI module for //' | sed "s/'//g"`
      if [ -n "$info" ]
      then
        echo $driver $info
      fi
    done
  ) | sort -u >$FILE

rm /tmp/$$filelist.tmp /tmp/$$driverlist.tmp
