#!/bin/sh
#----------------------------------------------------------------------------
# ipv6-restart-daemons.sh                                        __FLI4LVER__
#
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

. /etc/boot.d/base-helper

sync_lock_resource ipv6-restart ipv6-restart-daemons.sh

if [ -f /var/run/reload-legacy.radvd ]
then
    reload-radvd.sh
    rm /var/run/reload-legacy.radvd
fi
if [ -f /var/run/reload.dnsmasq ]
then
    [ -f /var/run/dnsmasq.pid ] && kill -HUP $(cat /var/run/dnsmasq.pid)
    # as the DNS configuration has changed, it is possible that some hosts
    # can now be mapped to valid IPv6 addresses now, so we have to rebuild
    # the dynamic rules
    setup-dynamic-ip6tables.sh
    rm /var/run/reload.dnsmasq
fi

sync_unlock_resource ipv6-restart ipv6-restart-daemons.sh
exit 0
