#! /bin/sh
#----------------------------------------------------------------------------
# /usr/local/bin/tun6in4-static-circuit-ctrl.sh                  __FLI4LVER__
#
# responsible for dialling and hanging up 6in4 tunnel circuits of type 'static'
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

. /usr/share/circuits/api

script="$(basename $0)[$$]"
facility=$circuit_logfacility
. /usr/share/logfunc.sh

dial()
{
    log_info "$circ_id: starting 6in4 tunnel of type $circ_tun6in4_type"
    ip -6 addr add $circ_tun6in4_static_localv6 dev $circ_dev

    /etc/ppp/ipv6-up $circ_dev /dev/null 0 "$circ_tun6in4_static_localv6" \
        "$circ_tun6in4_static_remotev6" $circ_id &

    if [ -n "$circ_tun6in4_static_prefix" ]
    then
        /etc/ppp/prefixv6-up $circ_dev $circ_id "$circ_tun6in4_static_prefix" &
    fi

    return 0
}

hangup()
{
    log_info "$circ_id: terminating 6in4 tunnel of type $circ_tun6in4_type"
    ip -6 addr del $circ_tun6in4_static_localv6 dev $circ_dev

    /etc/ppp/ipv6-down $circ_dev /dev/null 0 "$circ_tun6in4_static_localv6" \
        "$circ_tun6in4_static_remotev6" $circ_id &

    if [ -n "$circ_tun6in4_static_prefix" ]
    then
        /etc/ppp/prefixv6-down $circ_dev $circ_id \
            "$circ_tun6in4_static_prefix" &
    fi

    return 0
}

case $1 in
    dial|hangup) $1 ;;
    *)           log_error "unsupported command '$*'" ;;
esac
