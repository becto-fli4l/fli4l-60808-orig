#!/bin/sh
#----------------------------------------------------------------------------
# dibbler-notify.sh                                               __FLI4LVER__
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

script=$(basename $0)
circ_id=${script#dibbler-notify-*}
circ_id=${circ_id%*.sh}

maxprefixvalidity=4294967295

i=0
while true
do
    i=$((i+1))
    eval prefix=\$SRV_PREFIX$i
    eval prefixlen=\$SRV_PREFIX${i}LEN
    [ -n "$prefix" -a -n "$prefixlen" ] || break

    eval valid_lft=\$SRV_PREFIX${i}VALID
    eval preferred_lft=\$SRV_PREFIX${i}PREF

    if [ "$valid_lft" = $maxprefixvalidity ]
    then
        valid_lft=forever
    fi
    if [ "$preferred_lft" = $maxprefixvalidity ]
    then
        preferred_lft=forever
    fi

    case $1 in
    add|update)
        /etc/ppp/prefixv6-up $IFACE $circ_id $prefix/$prefixlen $valid_lft $preferred_lft &
        ;;
    delete)
        /etc/ppp/prefixv6-down $IFACE $circ_id $prefix/$prefixlen &
        ;;
    *)
        echo "ignoring unknown operation '$1' for prefix $prefix received on interface $IFACE"
        ;;
    esac
done
[ -n "$CIRCD_SYNC" ] && wait

exit 0
