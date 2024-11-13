#!/bin/sh
#----------------------------------------------------------------------------
# /etc/boot.d/netalias.inc
# Functions for managing network aliases.
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

if [ "$netalias_api" != yes ]
then

. /etc/boot.d/string.inc
. /etc/boot.d/locking.inc

: ${net_alias_conf:=/var/run/netaliases.conf}
#
# net_alias_add name device [ prefix ]
#
net_alias_add()
{
    local name="$1"
    local dev="$2"
    local pf="$3"
    local rname=$(to_shell_id "$name")
    local rdev=$(to_shell_id "$dev")

    eval "net_${pf}dev_$rname=\"$dev\""
    eval "net_${pf}name_$rdev=\"$name\""
    sync_lock_resource net_alias_db net_alias_add
    {
        echo "net_${pf}dev_$rname='$dev'"
        echo "net_${pf}name_$rdev='$name'"
    } >>$net_alias_conf
    sync_unlock_resource net_alias_db net_alias_add
}
net_alias_del()
{
    local rname=$(to_shell_id "$1")
    sync_lock_resource net_alias_db net_alias_del
    sed -i -e "/^net_$2dev_$rname=/d;/^net_$2name_.*='$1'/d" $net_alias_conf
    sync_unlock_resource net_alias_db net_alias_del
}
net_alias_lookup()
{
    [ -f $net_alias_conf ] || return 1
    local postfix=$(to_shell_id "$2")
    local name="net_$4$1_$postfix"
    sync_lock_resource net_alias_db net_alias_lookup
    eval local $(grep "^$name=" $net_alias_conf)
    sync_unlock_resource net_alias_db net_alias_lookup
    eval local tmp=\"\$$name\"
    if [ "$tmp" ]; then
       eval $3="\"$tmp\""
       return 0
    fi
    return 1
}
net_alias_lookup_dev()
{
    net_alias_lookup dev "$1" $2 $3
}
net_alias_lookup_name()
{
    net_alias_lookup name "$1" $2 $3
}

netalias_api='yes'

fi # $netalias_api != yes
