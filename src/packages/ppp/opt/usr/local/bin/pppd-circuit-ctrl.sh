#! /bin/sh
#----------------------------------------------------------------------------
# /usr/local/bin/pppd-circuit-ctrl.sh                            __FLI4LVER__
#
# responsible for dialling and hanging up pppd circuits
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

. /usr/share/circuits/api

script="$(basename $0)[$$]"
facility=$circuit_logfacility
. /usr/share/logfunc.sh

dial()
{
    local dialmode=$(circuit_get_effective_dialmode $circ_id)
    local peer_file=$ppp_peers_dir/circuit.$circ_id.$dialmode
    if [ -f $peer_file ]
    then
        circuit_daemon_wrapper $circ_id pppd "" call circuit.$circ_id.$dialmode
    else
        log_error "cannot dial over $circ_id due to invalid dial mode $dialmode"
    fi
}

hangup()
{
    # we can only terminate a dial-on-demand link reliably if we kill the
    # pppd; sending SIGHUP does nothing if no link has been established yet
    # at least once
    # => it follows that this is another reason why dial-on-demand requires
    # a fixed master circuit and not a floating one (the first one being
    # routing: the initial packets triggering the dial need to go to a
    # well-known device which happens to be that one of the master pppd)
    local sig=
    if [ $circ_hup_timeout -eq 0 ]
    then
        sig=HUP
    fi

    circuit_daemon_terminate $circ_id $sig
}

case $1 in
    dial|hangup) $1 ;;
    *)           log_error "unsupported command '$*'" ;;
esac
