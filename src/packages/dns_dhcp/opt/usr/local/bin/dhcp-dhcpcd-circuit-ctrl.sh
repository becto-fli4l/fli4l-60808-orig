#! /bin/sh
#----------------------------------------------------------------------------
# /usr/local/bin/dhcp-dhcpcd-circuit-ctrl.sh                     __FLI4LVER__
#
# responsible for dialling and hanging up DHCPv4 circuits (dhcpcd)
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

. /etc/boot.d/base-helper
. /usr/share/circuits/api

script="$(basename $0)[$$]"
facility=$circuit_logfacility
. /usr/share/logfunc.sh

dial()
{
    local options= hostname= debug=
    case $circ_dhcp_accept_csr in
    yes)
        options="${options:+$options,}classless_static_routes,ms_classless_static_routes"
        ;;
    esac
    case $circ_usepeerdns in
    yes)
        options="${options:+$options,}domain_name,domain_name_servers"
        ;;
    esac

    [ "$circ_dhcp_hostname" ] && hostname="-h $circ_dhcp_hostname"
    [ $circ_debug -gt 0 ] && debug=-d

    log_info "$circ_id: starting dhcpcd"
    circuit_daemon_wrapper $circ_id dhcpcd "" -B -e circ_id=$circ_id $debug \
        -c /etc/dhcpcd.sh -G -L -b --noipv6 \
        $hostname ${options:+-o $options} $circ_dev
}

hangup()
{
    log_info "$circ_id: terminating dhcpcd"
    dhcpcd -k $circ_dev 2>&1 | logmsg $script $facility.notice
}

case $1 in
    dial|hangup) $1 ;;
    *)           log_error "unsupported command '$*'" ;;
esac
