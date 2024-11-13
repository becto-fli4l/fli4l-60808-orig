#! /bin/sh
#----------------------------------------------------------------------------
# /usr/local/bin/dhcp-dibbler-circuit-ctrl.sh                    __FLI4LVER__
#
# responsible for dialling and hanging up DHCPv6 circuits (dibbler)
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

. /usr/share/circuits/api

script="$(basename $0)[$$]"
facility=$circuit_logfacility
. /usr/share/logfunc.sh

dial()
{
    local dibbler_dir=/var/lib/dibbler/$circ_id
    mkdir -p $dibbler_dir

    local config_file=$dibbler_dir/client.conf
    local notify_script=$dibbler_dir/dibbler-notify-${circ_id}.sh

    cat > $config_file <<EOF
log-mode syslog
log-name "-$circ_id"
log-level 8
script "$notify_script"
iface $circ_dev {
    pd {
EOF

    case $circ_dhcp_prefix_length in
    '') echo "        prefix";;
    *)  echo "        prefix ::/$circ_dhcp_prefix_length";;
    esac >> $config_file

    cat >> $config_file <<EOF
    }
}
EOF

    ln -sf /usr/local/bin/dibbler-notify.sh $notify_script

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
        echo 0 > /proc/sys/net/ipv6/conf/$circ_dev/disable_ipv6
        ip link set dev $circ_dev up

        # wait a bit to get at least a tentative link-local address on the
        # interface
        sleep 2

        # we need to receive router advertisements (RAs) on the link in order
        # to install a default route to the default gateway
        local dhcp_old_accept_ra=1
        read dhcp_old_accept_ra < /proc/sys/net/ipv6/conf/$circ_dev/accept_ra
        echo 2 > /proc/sys/net/ipv6/conf/$circ_dev/accept_ra
        circuit_write_field $circ_id dhcp_old_accept_ra $dhcp_old_accept_ra

        # perform router solicitation to receive a router advertisement faster
        rdisc6 -1 -w 1000 $circ_dev
        ;;
    esac

    circuit_daemon_wrapper $circ_id \
        dibbler-client dhcp_circuit_cleanup run -w $dibbler_dir
}

hangup()
{
    circuit_daemon_terminate $circ_id

    # restore accept_ra setting if possible
    local accept_ra=/proc/sys/net/ipv6/conf/$circ_dev/accept_ra
    if [ -w $accept_ra ]
    then
        echo ${dhcp_old_accept_ra:-1} > $accept_ra
    fi
}

case $1 in
    dial|hangup) $1 ;;
    *)           log_error "unsupported command '$*'" ;;
esac
