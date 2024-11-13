#! /bin/sh
#----------------------------------------------------------------------------
# /usr/local/bin/dhcp-server-dibbler-circuit-ctrl.sh             __FLI4LVER__
#
# responsible for dialling and hanging up DHCPv6 server circuits (dibbler)
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

. /usr/share/circuits/api

script="$(basename $0)[$$]"
facility=$circuit_logfacility
. /usr/share/logfunc.sh

dial()
{
    local dhcp_dev=$(circuit_get_interface $circ_dhcp_server_dev)
    local dhcp_match="$dhcp_dev"
    case $circ_multi in
    yes)
        dhcp_match="$dhcp_match$circ_multi_ifsuffix+"
        ;;
    esac

    if [ $circ_dhcp_server_prefix_pool_n -gt 0 ]
    then
        local dibbler_dir=/var/lib/dibbler/$circ_id
        mkdir -p $dibbler_dir

        local config_file=$dibbler_dir/server.conf

        cat > $config_file <<EOF
log-mode syslog
log-name "-$circ_id"
log-level 8
inactive-mode
iface "$dhcp_match" {
EOF

        local i num_pools=0
        for i in $(seq 1 $circ_dhcp_server_prefix_pool_n)
        do
            eval local network=\$circ_dhcp_server_prefix_pool_${i}_network
            eval local prefixlen=\$circ_dhcp_server_prefix_pool_${i}_prefix_length
            local pool=$(circuit_resolve_address "$network")
            if [ -n "$pool" ]
            then
                cat >> $config_file <<EOF
    pd-class {
        pd-pool $pool
        pd-length $prefixlen
    }
EOF
                num_pools=$((num_pools+1))
            else
                log_warn "$circ_id: cannot translate prefix pool $network, ignoring"
            fi
        done

        if [ $num_pools -eq 0 ]
        then
            log_error "$circ_id: no usable prefix pools"
            rm $config_file
            return 1
        fi

        cat >> $config_file <<EOF
}
EOF

        # Dibbler requires us to ensure that the interface is up and has an
        # IPv6 link-local address. However, we do this _only_ when our interface
        # is fixed and not dynamically created by some other component (e.g.
        # the PPP daemon).
        #
        # NOTE that this is a temporary hack -- we need a proper layer-2
        # interface management layer for this... For the same reason, we do
        # _not_ put the interface into DOWN state when hanging up this circuit
        # as we do not know whether there are other circuits operating on the
        # same interface.
        case $circ_newif in
        no)
            echo 0 > /proc/sys/net/ipv6/conf/$dhcp_dev/disable_ipv6
            ip link set dev $dhcp_dev up

            # wait a bit to get at least a optimistic link-local address on the
            # interface
            sleep 2
            ;;
        esac

        dibbler-server start -w $dibbler_dir
        return 0

        # remain in state 'ready' as this is a server circuit
    else
        log_error "$circ_id: no prefix pools configured"
        return 1
    fi
}

hangup()
{
    local dibbler_dir=/var/lib/dibbler/$circ_id
    dibbler-server stop -w $dibbler_dir
    circuit_event_id=$circ_id \
        down_link_circuit_event_force_hangup= \
        mom_unicast circd down_link_circuit_event
}

case $1 in
    dial|hangup) $1 ;;
    *)           log_error "unsupported command '$*'" ;;
esac
