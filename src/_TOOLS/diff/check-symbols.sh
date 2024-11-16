#! /bin/sh

#  * Broken Drivers:
#    * Known broken MOD_PARAM deklaration
#      opt/lib/modules/2.4.32/kernel/drivers/net/3c59x-db.o
#      opt/lib/modules/2.4.32/kernel/drivers/net/arlan.o
#      opt/lib/modules/2.4.32/kernel/drivers/net/via-rhine-db.o
#
#    * Unknown Reason (not checked yet)
#      opt/lib/modules/2.6.16.27/kernel/drivers/hwmon/hdaps.ko
#      opt/lib/modules/2.6.16.27/kernel/drivers/isdn/capi/capi.ko
#      opt/lib/modules/2.6.16.27/kernel/drivers/media/dvb/b2c2/b2c2-flexcop.ko
#      opt/lib/modules/2.6.16.27/kernel/drivers/media/dvb/dvb-core/dvb-core.ko
#      opt/lib/modules/2.6.16.27/kernel/drivers/media/dvb/dvb-usb/dvb-usb-cxusb.ko
#      opt/lib/modules/2.6.16.27/kernel/drivers/media/dvb/dvb-usb/dvb-usb-digitv.ko
#      opt/lib/modules/2.6.16.27/kernel/drivers/media/dvb/dvb-usb/dvb-usb-dtt200u.ko
#      opt/lib/modules/2.6.16.27/kernel/drivers/media/dvb/dvb-usb/dvb-usb.ko
#      opt/lib/modules/2.6.16.27/kernel/drivers/media/dvb/dvb-usb/dvb-usb-vp702x.ko
#      opt/lib/modules/2.6.16.27/kernel/drivers/media/dvb/dvb-usb/dvb-usb-vp7045.ko
#      opt/lib/modules/2.6.16.27/kernel/drivers/media/dvb/ttpci/budget-core.ko
#      opt/lib/modules/2.6.16.27/kernel/drivers/media/dvb/ttpci/dvb-ttpci.ko
#      opt/lib/modules/2.6.16.27/kernel/drivers/net/wireless/arlan.ko
#      opt/lib/modules/2.6.16.27/kernel/drivers/scsi/scsi_mod.ko
#      opt/lib/modules/2.6.16.27/kernel/drivers/scsi/sg.ko
#      opt/lib/modules/2.6.16.27/kernel/drivers/scsi/sym53c8xx_2/sym53c8xx.ko
#      opt/lib/modules/2.6.16.27/kernel/drivers/usb/input/usbhid.ko
#      opt/lib/modules/2.6.16.27/kernel/drivers/usb/storage/libusual.ko
#      opt/lib/modules/2.6.16.27/kernel/drivers/w1/wire.ko

MI=/sbin/modinfo
exec 3> log

if [ ! -e files.txt ]
then
    echo "can't open files.txt"
    exit 1
fi

tmp_file=/tmp/check_symbols.$$

for i in `grep	-e '^[b].*opt/lib/modules/' files.txt | sed "s/. //" `
do
    nm $i > $tmp_file

    $MI $i 1>&3
    case $i in 
	*/2.4*)
	    symbols="`$MI $i | grep ^parm_ | grep -v ^parm_desc |\
		sed -e 's/^parm_\([^:]*\):.*/\1/'`"
	    ;;
	*)
	    symbols="`$MI $i | \
		grep ^parm: | sed -e 's/^parm:[[:space:]]\([^:]*\):.*/\1/'`"
	    ;;
    esac
    if [ -n "$symbols" ]; then
	for s in $symbols; do
	    if ! grep -w $s $tmp_file 1>&3; then
		echo "$i: missing symbol $s"
	    fi
	done
    fi
  
done
exit 0
