#! /bin/sh
#----------------------------------------------------------------------------
# /usr/local/bin/ppp-pptp-server-circuit-ctrl.sh                 __FLI4LVER__
#
# responsible for controlling PPTP server circuits
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

. /usr/share/circuits/api

script="$(basename $0)[$$]"
facility=$circuit_logfacility
. /usr/share/logfunc.sh

dial()
{
    circuit_daemon_wrapper $circ_id pptpd "" \
        -f -e /usr/local/bin/pptp_pppd_wrapper -c $pptpd_conf_dir/$circ_id.conf

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
