#! /bin/sh
# ----------------------------------------------------------------------------
# /etc/dhcpcd.sh - invoked by DHCP daemon
#
# Creation:     23.01.2002  od
# Last Update:  $Id$
#----------------------------------------------------------------------------

. /usr/share/circuits/api

dhcp_scripts ()
{
    local event=$1
    logmsg dhcp-client daemon.notice "Event: $event, local: '$local', remote: '$remote'"
    tty=
    speed=

    case $event in
    up)
        /etc/ppp/ip-up "$interface" "$tty" "$speed" "$local" "$remote" "$circ_id"
        ;;
    down)
        /etc/ppp/ip-down "$interface" "$tty" "$speed" "$local" "$remote" "$circ_id"
        ;;
    esac
}

parse_info ()
{
    local type=$1
    remote=

    eval local subnet_mask=\$${type}_subnet_mask
    eval local=\$${type}_ip_address
    if [ -n "$subnet_mask" ]
    then
        local=$local/$(netcalc netmaskbits $subnet_mask)
    fi
    eval local classless_static_routes="\$${type}_classless_static_routes"
    eval local ms_classless_static_routes="\$${type}_ms_classless_static_routes"
    eval local routers="\$${type}_routers"
    eval local domain_name=\$${type}_domain_name
    eval local domain_name_servers="\$${type}_domain_name_servers"

    # first try CSRs (classless static routes) according to RFC 3442
    : ${classless_static_routes:=$ms_classless_static_routes}
    if [ -n "$classless_static_routes" ]
    then
        local gw=
        set -- $classless_static_routes
        while [ -n "$2" ]
        do
            if [ -z "$gw" ]
            then
                gw="$2"
            elif [ "$gw" != "$2" ]
            then
                gw=
                break
            fi
            shift 2
        done
        # set remote gateway if _all_ gateways that have been received using
        # the classless-static-routes option are equal
        [ -n "$gw" ] && remote=$gw
    elif [ -n "$routers" ]
    then
        set -- $routers
        remote=$1
    fi

    local oIFS="$IFS"
    IFS=","
    set -- $domain_name_servers
    IFS="$oIFS"

    DNS1=$1
    DNS2=$2
    DNSDOMAIN=$domain_name
    export DNS1 DNS2 DNSDOMAIN
}

run_dhcp ()
{
    logmsg dhcp-client daemon.info "DHCP event '$reason'"
    set | grep -e ^new -e ^old

    local vars=$(circuit_get_data $circ_id)
    local rc=$?
    [ $rc -eq 0 -a -n "$vars" ] || return $rc
    eval export $vars
    [ $circ_debug -gt 0 ] && set -x

    case $reason in
        PREINIT)
            if [ -n "$circ_dhcp_startdelay" ]
            then
                logmsg dhcp-client daemon.info "    sleeping $circ_dhcp_startdelay seconds"
                sleep $circ_dhcp_startdelay
            fi
            ;;
        NOCARRIER|RELEASE|EXPIRE|NAK|FAIL|STOP)
            parse_info old
            [ -n "$local$remote" ] && dhcp_scripts down
            ;;
        REBOOT)
            parse_info new
            [ -n "$local$remote" ] && dhcp_scripts up
            ;;
        BOUND|RENEW|REBIND|IPV4LL)
            if [ "$old_ip_address" != "$new_ip_address" -o \
                    "$new_routers" != "$old_routers" -o \
                    "$new_domain_name" != "$old_domain_name" -o \
                    "$new_domain_name_servers" != "$old_domain_name_servers" ]
            then
                parse_info old
                [ -n "$local$remote" ] && dhcp_scripts down
                parse_info new
                [ -n "$local$remote" ] && dhcp_scripts up
            fi
            ;;
        *)
            logmsg dhcp-client daemon.info "    ignoring dhcp event '$reason'"
            ;;
    esac
}

# kristov: use well-defined PATH
export PATH=/usr/local/sbin:/usr/sbin:/sbin:/usr/local/bin:/usr/bin:/bin
run_dhcp $* 2>&1 | logmsg dhcp-client daemon.debug
