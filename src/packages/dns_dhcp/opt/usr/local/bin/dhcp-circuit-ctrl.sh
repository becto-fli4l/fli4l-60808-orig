#! /bin/sh
#----------------------------------------------------------------------------
# /usr/local/bin/dhcp-circuit-ctrl.sh                            __FLI4LVER__
#
# responsible for dialling and hanging up DHCP circuits
# delegates to a daemon-specific control script
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

. /usr/share/circuits/api

script="$(basename $0)[$$]"
facility=$circuit_logfacility
. /usr/share/logfunc.sh

daemon_script=dhcp-${circ_dhcp_daemon}-circuit-ctrl.sh
if type -p $daemon_script >/dev/null
then
    $daemon_script "$@"
else
    log_error "cannot execute $daemon_script"
fi
