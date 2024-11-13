#!/bin/sh
#----------------------------------------------------------------------------
# /usr/local/bin/setup-dynamic-ip6tables.sh - setup dynamic IPv6 packet filter
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

[ -f /var/run/ip6_net.conf ] || exit 0

. /etc/boot.d/base-helper
. /etc/rc.d/fwrules-helper
. /etc/rc.d/fwrules-helper.ipv6

handle_chain()
{
    local table=$1 chain=$2 method=$3

    local processed=
    local delta=0
    local added=0
    ip6tables -t $table -nL $chain --line-numbers | grep "$dummy_comment \*/" | while read pos rest
    do
        if [ -n "$processed" ]
        then
            ip6tables -t $table -D $chain $((pos+added-delta))
            delta=$((delta+1))
        else
            do_rule6 "$table" "$chain" R "$rule" $pos $method "$comment"
            added=$((ip6tables_rules_added-1))
        fi
        processed=1
    done
}

sync_lock_resource ip6tables setup-dynamic-ip6tables.sh

for i in $(seq 1 $(cat $ip6tables_dynrules_idx))
do
    rulefile=$ip6tables_dynrules/$i.rule
    [ -f "$rulefile" ] || continue
    . "$rulefile"

    handle_chain $table $chain exec_rule6
    case $chain in
    PORTFW)
        handle_chain filter PORTFWACCESS exec_access_rule6
        handle_chain filter PORTREDIRACCESS exec_access_rule6
        ;;
    esac
done

sync_unlock_resource ip6tables setup-dynamic-ip6tables.sh
