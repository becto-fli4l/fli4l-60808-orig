#! /bin/sh
if [ "$1" ]; then
        ver=$1
        if [ ! -d opt/lib/modules/$ver ]; then
                echo "can't find modules dir for version '$ver'"
                exit 1
        fi
else
        echo "usage: $0 kernel_version"
        exit 1
fi
mv ./opt/lib/modules/$ver/kernel/drivers/scsi/{BusL,busl}ogic.ko
mv ./opt/lib/modules/$ver/kernel/drivers/scsi/g_{NCR,ncr}5380.ko
for i in `find ./opt/lib/modules/$ver/kernel/net -name '*[A-Z]*'`
do
        name=`echo $i | tr 'A-Z' 'a-z' | sed -e 's/\.ko/.target.ko/'`
        mv $i $name
done

