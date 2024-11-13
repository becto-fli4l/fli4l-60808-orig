#! /bin/sh
#----------------------------------------------------------------------------
# /usr/local/bin/ppp-server-circuit-ctrl.sh                      __FLI4LVER__
#
# responsible for controlling simple PPP server circuits, i.e. server
# circuits which start one instance of pppd
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

. /usr/share/circuits/api

script="$(basename $0)[$$]"
facility=$circuit_logfacility
. /usr/share/logfunc.sh

dial()
{
    circuit_daemon_wrapper $circ_id ppp-single-server "" $circ_id
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
