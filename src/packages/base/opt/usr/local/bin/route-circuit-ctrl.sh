#! /bin/sh
#----------------------------------------------------------------------------
# /usr/local/bin/route-circuit-ctrl.sh                           __FLI4LVER__
#
# responsible for dialling and hanging up route circuits
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

. /etc/boot.d/list.inc
. /etc/boot.d/networking.inc

script="$(basename $0)[$$]"
facility=$circuit_logfacility
. /usr/share/logfunc.sh

dial()
{
    local i

    eval local $(grep "^\(IP_NET_N\|IP_NET_[0-9]\+\(_DEV\)\?\)=" /etc/rc.cfg)
    eval local $(grep "^\(IPV6_NET_N\|IPV6_NET_[0-9]\+\(_DEV\)\?\)=" /etc/rc.cfg)

    route_local_ipv4=
    route_local_ipv6=

    if [ -n "$circ_dev" ]
    then
        route_local_ipv4_dev=$circ_dev
        route_local_ipv6_dev=$circ_dev
    fi

    if [ -n "$circ_route_gateway_ipv4" -a -z "$circ_dev" ]
    then
        translate_ipv4_to_dev $circ_route_gateway_ipv4 route_local_ipv4_dev || return 1
    fi

    if [ -n "$circ_route_gateway_ipv6" -a -z "$circ_dev" ]
    then
        translate_ipv6_to_dev $circ_route_gateway_ipv6 route_local_ipv6_dev || return 1
    fi

    if translate_dev_to_ipv4 $route_local_ipv4_dev route_local_ipv4 && [ -n "$route_local_ipv4" ]
    then
        # TODO: handle multiple addresses assigned to an interface
        route_local_ipv4=${route_local_ipv4%% *}
    fi
    if [ "$OPT_IPV6" = "yes" ] && translate_dev_to_ipv6 $route_local_ipv6_dev route_local_ipv6 && [ -n "$route_local_ipv6" ]
    then
        # TODO: handle multiple addresses assigned to an interface
        route_local_ipv6=${route_local_ipv6%% *}
    fi

    local route_resolved_gateway_ipv4=$circ_route_gateway_ipv4
    if [ -n "$circ_route_gateway_ipv4" ]
    then
        translate_ip_net $circ_route_gateway_ipv4 ${circ_id}_gateway_ipv4 route_resolved_gateway_ipv4 || return 1
        route_resolved_gateway_ipv4=$(list_last $route_resolved_gateway_ipv4)
        route_resolved_gateway_ipv4=${route_resolved_gateway_ipv4%/*}
    fi
    local route_resolved_gateway_ipv6=$circ_route_gateway_ipv6
    if [ -n "$circ_route_gateway_ipv6" ]
    then
        translate_ip6_net $circ_route_gateway_ipv6 ${circ_id}_gateway_ipv6 route_resolved_gateway_ipv6 || return 1
        route_resolved_gateway_ipv6=$(list_last $route_resolved_gateway_ipv6)
        route_resolved_gateway_ipv6=${route_resolved_gateway_ipv6%/*}
    fi

    circuit_write_field $circ_id route_local_ipv4 "$route_local_ipv4"
    circuit_write_field $circ_id route_resolved_gateway_ipv4 "$route_resolved_gateway_ipv4"

    circuit_write_field $circ_id route_local_ipv6 "$route_local_ipv6"
    circuit_write_field $circ_id route_resolved_gateway_ipv6 "$route_resolved_gateway_ipv6"

    if [ $circ_nets_ipv4_n -gt 0 ]
    then
        /etc/ppp/ip-up $circ_dev /dev/null 0 "$route_local_ipv4" "$route_resolved_gateway_ipv4" $circ_id &
    fi

    if [ "$OPT_IPV6" = "yes" ]
    then
        if [ $circ_nets_ipv6_n -gt 0 ]
        then
            /etc/ppp/ipv6-up $circ_dev /dev/null 0 "$route_local_ipv6" "$route_resolved_gateway_ipv6" $circ_id &
        fi
    fi

    return 0
}

hangup()
{
    if [ $circ_nets_ipv4_n -gt 0 ]
    then
        local route_local_ipv4
        local route_resolved_gateway_ipv4
        circuit_read_field $circ_id route_local_ipv4
        circuit_read_field $circ_id route_resolved_gateway_ipv4
        /etc/ppp/ip-down $circ_dev /dev/null 0 "$route_local_ipv4" "$route_resolved_gateway_ipv4" $circ_id &
    fi

    if [ $circ_nets_ipv6_n -gt 0 ]
    then
        local route_local_ipv6
        local route_resolved_gateway_ipv6
        circuit_read_field $circ_id route_local_ipv6
        circuit_read_field $circ_id route_resolved_gateway_ipv6
        /etc/ppp/ipv6-down $circ_dev /dev/null 0 "$route_local_ipv6" "$route_resolved_gateway_ipv6" $circ_id &
    fi

    circuit_write_field $circ_id route_local_ipv4 ""
    circuit_write_field $circ_id route_resolved_gateway_ipv4 ""

    circuit_write_field $circ_id route_local_ipv6 ""
    circuit_write_field $circ_id route_resolved_gateway_ipv6 ""

    return 0
}

case $1 in
    dial|hangup) $1 ;;
    *)           log_error "unsupported command '$*'" ;;
esac
