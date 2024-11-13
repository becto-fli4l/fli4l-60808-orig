#!/bin/sh
#----------------------------------------------------------------------------
# /usr/local/bin/newfax.sh - process received faxes              __FLI4LVER__
#
# Creation:     28.09.2003  felix
# Last Update:  $Id$
#----------------------------------------------------------------------------
# call with 'newfax.sh <index> <fax file>'

exec > /dev/null 2>&1 < /dev/null

idx=$1
shift

. /var/run/faxrcv.conf

eval dir='$FAXRCV_'$idx'_DIRECTORY'
eval cmd_n='$FAXRCV_'$idx'_NTFYCMD_N'
eval csid='$FAXRCV_'$idx'_CSID'

# compress fax
gzip < "$1" > "$1.sff.gz"

case $? in
    0)
        rm "$1"
        file="$1.sff.gz"
    ;;
    *)
        # remove the damaged file just to be sure
        rm -f "$1.sff.gz"
        # rename the file to make it recognizeable
        mv "$1" "$1.sff"
        file="$1.sff"
        # and output a warning
        facility="newfax.sh"
        message="ERROR gzipping the file $file! Probably not enough memory?"
        if [ -f /usr/bin/logger -o -L /usr/bin/logger ]
        then
            logger -t "$facility" "$message"
        else
            colecho "$facility: $message" br x br > /dev/console 2>&1
        fi
    ;;
esac

# add mfax.log entry
case x$dir in
    x) ;;
    *)
        SENDER=`echo $1 | cut -c 14-`
        PAGES=`echo $1 | cut -c 3-5 | sed 's/^[0]*//'`
        case x$SENDER in
            x) SENDER="?" ;;
        esac
        LANGDATUM=`date +%d.%m.%y`
        LANGZEIT=`date +%H:%M:%S`
        echo "$LANGDATUM $LANGZEIT $SENDER $PAGES $file" >> $dir/mfax.log
        case x"$FAXRCV_IMONC_DIR" in
            x) ;;
            *) echo "$LANGDATUM $LANGZEIT $SENDER $PAGES $FAXRCV_DIRECTORY/$file" >> $dir/mfax.log ;;
        esac
    ;;
esac

# set fax led
if [ -f /usr/local/bin/faxrcv_setleds ]
then 
    faxrcv_setleds blink
fi

# send mail
if [ -f /usr/local/bin/faxrcv-mail.sh ]
then
    faxrcv-mail.sh $idx $file
fi

# call user command
[ 0$cmd_n -eq 0 ] || for nidx in `seq 1 $cmd_n`
do
    eval ntfycmd='$FAXRCV_'$idx'_NTFYCMD'_$nidx
    eval "$ntfycmd" \"\$dir/\$file\" \"\$csid\"
done
