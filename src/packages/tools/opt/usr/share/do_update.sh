#!/bin/sh

export LC_ALL=C
for i in hw_detect.sed-update.sh hw_driverinfo-update.sh usb.ids-convert.sh
do
  echo running $i
  /bin/sh $i
done
