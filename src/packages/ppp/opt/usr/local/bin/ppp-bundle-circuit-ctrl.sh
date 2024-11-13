#! /bin/sh
#----------------------------------------------------------------------------
# /usr/local/bin/ppp-bundle-circuit-ctrl.sh                      __FLI4LVER__
#
# responsible for dialling and hanging up PPP bundle circuits
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

. /usr/share/circuits/api

script="$(basename $0)[$$]"
facility=$circuit_logfacility
. /usr/share/logfunc.sh

dial()
{
    # remain in state 'ready': the bundle goes 'online' together with the first
    # link
    :
}

hangup()
{
    local state=$(circuit_get_state $circ_id)
    case $state in
    ready)
        # no active links, we simply hangup the bundle circuit
        circuit_event_id=$circ_id \
            down_link_circuit_event_force_hangup= \
            mom_unicast circd down_link_circuit_event
        ;;
    *)
        local links=$(circuit_get_bundle_links $circ_id)
        if [ -n "$links" ]
        then
            local link circ_hup_timeout
            circuit_read_field $circ_id circ_hup_timeout

            if [ $circ_hup_timeout -ne 0 ]
            then
                # if this is a dial-on-demand bundle, it suffices to hangup the
                # master link circuit, as this leads to hangup of all slave link
                # circuits (it is also advisable to _not_ explicitly terminate
                # the slave circuits as it can happen that the pppd daemons
                # receive the termination signal twice, once by us and once by
                # the master circuit pppd, resulting in immediate termination
                # without waiting for completion of the ip-down/link-down
                # scripts)
                local master=$(bundle_map_bundle_circuit_to_master "$circ_id")
                if [ -n "$master" ]
                then
                    fli4lctrl hangup $master
                #else: the master circuit is obsolete, so the links of the
                #      bundle are already in the process of being hung up
                fi
            else
                # this is not a dial-on-demand bundle, simply hangup all link
                # circuits
                for link in $links
                do
                    fli4lctrl hangup $link
                done
            fi
        else
            # no links at all, we simply hangup the bundle circuit
            circuit_event_id=$circ_id \
                down_link_circuit_event_force_hangup= \
                mom_unicast circd down_link_circuit_event
        fi
        ;;
    esac
}

case $1 in
    dial|hangup) $1 ;;
    *)           log_error "unsupported command '$*'" ;;
esac
