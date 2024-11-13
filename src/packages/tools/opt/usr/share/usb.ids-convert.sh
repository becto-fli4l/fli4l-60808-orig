#!/bin/sh

# Download usb.ids and converts to script-readable form for hw.cgi

TMPFILE='/tmp/usb.ids.tmp'

VENDORFILE='usb.ids.vendor.sed'
DEVICEFILE='usb.ids.device.sed'
CLASSFILE='usb.ids.class.sed'
SUBCLASSFILE='usb.ids.subclass.sed'
PROTOCOLFILE='usb.ids.protocol.sed'

>$VENDORFILE
>$DEVICEFILE
>$CLASSFILE
>$SUBCLASSFILE
>$PROTOCOLFILE

MODE="DEVICE"
wget -qO $TMPFILE http://www.linux-usb.org/usb.ids
if [ ! -s "$TMPFILE" ]
then
  # Alternatively fetch from systemd repo
  wget -qO $TMPFILE https://github.com/systemd/systemd/raw/master/hwdb/usb.ids
fi
if [ -s "$TMPFILE" ]
then
  cat $TMPFILE |\
  sed 's/^\([0-9a-fA-F]\)/1: \1/;s/^\t\([0-9a-fA-F]\{2,4\}\)/2: \1/;s/^\t\t\([0-9a-fA-F]\)/3: \1/' |\
  while read a b c d
  do
    d=`echo $d | sed 's/[ \t]*$//g'`
    case $b in
      C) MODE="CLASS";;
      AT) MODE="AUDIOCLASS";;
    esac
    case $a in
      1:*)
        if [ "$MODE" = "DEVICE" ]
        then
          VENDORID=`echo $b | sed 'y/abcdef/ABCDEF/'`
          VENDORNAME=`echo $c $d | sed 's/[ \t]*$//g' | sed 's#/#\\\/#g'`
          echo "s/^usb:v${VENDORID}p.*/$VENDORNAME/p" >>$VENDORFILE
        else
          if [ "$MODE" = "CLASS" ]
          then
     CLASS=`echo $c | sed 'y/abcdef/ABCDEF/'`
     CLASSNAME=`echo $d | sed 's/[ \t]*$//g' | sed 's#/#\\\/#g'`
     echo "s/^usb:.*ic${CLASS}isc.*/$CLASSNAME/p" >>$CLASSFILE
          fi    
        fi
        ;;
      2:*)
        if [ "$MODE" = "DEVICE" ]
        then
          DEVICEID=`echo $b | sed 'y/abcdef/ABCDEF/'`
          DEVICENAME=`echo $c $d | sed 's/[ \t]*$//g' | sed 's#/#\\\/#g'`
          echo "s/^usb:v${VENDORID}p${DEVICEID}d.*/$DEVICENAME/p" >>$DEVICEFILE
        else
          if [ "$MODE" = "CLASS" ]
          then
     SUBCLASS=`echo $b | sed 'y/abcdef/ABCDEF/'`
     SUBCLASSNAME=`echo $c $d | sed 's/[ \t]*$//g' | sed 's#/#\\\/#g'`
     echo "s/^usb:.*ic${CLASS}isc${SUBCLASS}ip.*/$SUBCLASSNAME/p" >>$SUBCLASSFILE
          fi    
        fi
        ;;
      3:*)
        if [ "$MODE" = "CLASS" ]
        then
     PROTOCOL=`echo $b | sed 'y/abcdef/ABCDEF/'`
     PROTOCOLNAME=`echo $c $d | sed 's/[ \t]*$//g' | sed 's#/#\\\/#g'`
     echo "s/^usb:.*ic${CLASS}isc${SUBCLASS}ip${PROTOCOL}$/$PROTOCOLNAME/p" >>$PROTOCOLFILE
        fi
        ;;
    esac
  done

else
  echo "Error fetching USB IDs"
  exit 1
fi

rm -f $TMPFILE
