#! /bin/sh
#----------------------------------------------------------------------------
# /usr/local/bin/ppp-pppoe-server-circuit-ctrl.sh                __FLI4LVER__
#
# responsible for controlling PPPoE server circuits
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

. /usr/share/circuits/api

script="$(basename $0)[$$]"
facility=$circuit_logfacility
. /usr/share/logfunc.sh

dial()
{
    local pppoe_server_options=
    case "$circ_ppp_ethernet_server_type" in
    kernel)
        pppoe_server_options="$pppoe_server_options -k"
        ;;
    esac

    local circ_ppp_name circ_ppp_localip circ_ppp_remoteip

    if [ -n "$circ_bundle" ]
    then
        circuit_read_field $circ_bundle circ_ppp_name
    else
        circuit_read_field $circ_id circ_ppp_name
    fi
    pppoe_server_options="$pppoe_server_options -S \"$circ_ppp_name\""

    circuit_read_field $circ_id circ_ppp_localip
    [ -n "$circ_ppp_localip" ] &&
        pppoe_server_options="$pppoe_server_options -L $circ_ppp_localip"

    circuit_read_field $circ_id circ_ppp_remoteip
    [ -n "$circ_ppp_remoteip" ] &&
        pppoe_server_options="$pppoe_server_options -R $circ_ppp_remoteip"

    circuit_daemon_wrapper $circ_id pppoe-server "" \
        -I $circ_ppp_ethernet_server_dev -N $circ_ppp_ethernet_server_sessions \
        -O $circ_id -q /usr/local/bin/pppoe_pppd_wrapper -F $pppoe_server_options

    # remain in state 'ready' as this is a server circuit
}

hangup()
{
    circuit_daemon_terminate $circ_id
}

case $1 in
    dial|hangup) $1 ;;
    *)           log_error "unsupported command '$*'" ;;
esac
