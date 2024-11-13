#! /bin/sh
#----------------------------------------------------------------------------
# /usr/local/bin/tun6in4-circuit-ctrl.sh                         __FLI4LVER__
#
# responsible for dialling and hanging up 6in4 tunnel circuits
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

. /usr/share/circuits/api

script="$(basename $0)[$$]"
facility=$circuit_logfacility
. /usr/share/logfunc.sh

control_script=tun6in4-${circ_tun6in4_type}-circuit-ctrl.sh
if type -p $control_script >/dev/null
then
    $control_script "$@"
else
    log_error "$circ_id: no circuit control script for type $circ_tun6in4_type"
    exit 1
fi
