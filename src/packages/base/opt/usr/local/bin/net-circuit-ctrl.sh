#! /bin/sh
#----------------------------------------------------------------------------
# /usr/local/bin/net-circuit-ctrl.sh                             __FLI4LVER__
#
# responsible for dialling and hanging up net circuits
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

. /usr/share/circuits/api

script="$(basename $0)[$$]"
facility=$circuit_logfacility
. /usr/share/logfunc.sh

dial()
{
    if [ -n "$circ_net_ipv4_address$circ_net_ipv6_address" ]
    then
        ip link set dev $circ_dev up

        if [ -n "$circ_net_ipv4_address" ]
        then
            ip link set dev $circ_dev arp on
            net_circuit_change $circ_id ipv4 up
        fi
        if [ -n "$circ_net_ipv6_address" ]
        then
            echo 0 > /proc/sys/net/ipv6/conf/$circ_dev/disable_ipv6
            net_circuit_change $circ_id ipv6 up
        fi

        return 0
    else
        return 1
    fi
}

hangup()
{
    if [ -n "$circ_net_ipv4_address$circ_net_ipv6_address" ]
    then
        cleanup=1
        if [ -n "$circ_net_ipv4_address" ]
        then
            net_circuit_change $circ_id ipv4 down && unset cleanup
            ip link set dev $circ_dev arp off
        fi
        if [ -n "$circ_net_ipv6_address" ]
        then
            net_circuit_change $circ_id ipv6 down && unset cleanup
            echo 1 > /proc/sys/net/ipv6/conf/$circ_dev/disable_ipv6
        fi

        ip link set dev $circ_dev down

        # If no addresses were associated with this circuit at hangup time,
        # addr(v6)-down and consequently ip(v6)-down were not called, leaving
        # the circuit in the "ready" state. In order to perform the transition
        # out of the "ready" state, we signalize the circd that the link is
        # down.
        if [ -n "$cleanup" ]
        then
            circuit_event_id=$circ_id \
            down_link_circuit_event_force_hangup= \
                mom_unicast_message circd down_link_circuit_event >/dev/null
        fi
    fi

    return 0
}

case $1 in
    dial|hangup) $1 ;;
    *)           log_error "unsupported command '$*'" ;;
esac
