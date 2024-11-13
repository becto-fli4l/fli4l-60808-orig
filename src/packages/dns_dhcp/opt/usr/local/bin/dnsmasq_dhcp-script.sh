#!/bin/sh
#------------------------------------------------------------------------------
# /usr/local/bin/dnsmasq_dhcp-script.sh - handle dhcp-events
#
# Creation:     2014-09-30  LanSpezi
# Last Update:  $Id$
#------------------------------------------------------------------------------

op="${1}"
mac="${2}"
ip="${3}"
hostname="${4}"

case ${op} in
    add)
        ;;
    old)
        ;;
    del)
        if grep -q ${mac} /proc/net/arp
        then
            arp -d ${ip}
            logger -t dnsmasq_dhcp-script -p local0.info "dhcp-event: release - ${hostname} [${mac}] / ${ip} removed from arp table"
        fi
        ;;
esac
