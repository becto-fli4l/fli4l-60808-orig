#!/bin/sh
#----------------------------------------------------------------------------
# /etc/boot.d/networking.inc
# Functions for networking.
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

if [ "$networking_api" != yes ]
then

. /etc/boot.d/netalias.inc
. /etc/boot.d/logging.inc

# as parts of base-helper may depend on the circuit system, we include here
# circuit-common such that every user of base-helper also includes the
# functions dealing with circuits;
# as the API is not available in the rootfs the inclusion is guarded
# with a check, it is eventually included by rc.d/rc010.circuit.init
[ -f /usr/share/circuits/api ] && . /usr/share/circuits/api

#----------------------------------------------------------------------------
# Normalizes one or more subnet addresses by calling 'netcalc canonicalize'
# which e.g. removes leading zeros.
#
# $1... = subnet addresses in CIDR format
# output = canonicalized subnet address(es)
#----------------------------------------------------------------------------
normalize_networks()
{
    local _addr _res
    for _addr
    do
        normalize_network "$_addr"
        _res="$_res $res"
    done
    res=${_res# }
}

#----------------------------------------------------------------------------
# Normalizes a subnet address by calling 'netcalc canonicalize' which e.g.
# removes leading zeros.
#
# $1 = subnet address in CIDR format
# output = canonicalized subnet address
#----------------------------------------------------------------------------
normalize_network ()
{
    res=$1
    case $res in
        */*) res=`netcalc canonicalize $res` ;;
    esac
}

#----------------------------------------------------------------------------
# Checks if an interface can be translated statically. Returns 0 when result of
# translation is purely statically decidable and a nonzero value otherwise.
# A translation is statically undecidable in general if the interface is
# described by a dynamic alias.
#
# $1 = interface specification to translate
#----------------------------------------------------------------------------
is_static_net_if_hooks=
is_static_net_if()
{
    local hook
    for hook in $is_static_net_if_hooks
    do
        $hook $1 && return 0
    done

    case $1 in
    any|*+|*!)
        return 0
        ;;
    {*})
        # treat interfaces referenced by circuits always as dynamic; this can
        # be optimized later if necessary
        return 1
        ;;
    *)
        [ -d /sys/class/net/$1 ]
        ;;
    esac
}

#----------------------------------------------------------------------------
# Translates an interface specification (e.g. pppoe) into the real interface
# (e.g. ppp0).
#
# $1 = interface specification to translate
# $2 = name of variable receiving the translated value
# $3 = if nonzero, nonexisting interfaces are ignored (this is e.g. used for
#      the firewall, as the firewall accepts interface _expressions_ (e.g.
#      ppp+) and may reference interfaces that will be created later (e.g.
#      ppp interfaces dynamically created by pppoe_server)
#
# Returns a non-zero value if an unknown interface specification is passed.
#----------------------------------------------------------------------------
translate_net_if_hooks=
translate_net_if()
{
    local tid_tif=$1
    local tid_var=$2
    local tid_ignore=$3

    if [ ! "$tid_var" ]
    then
        log_error "translate_net_if: missing variable name"
        return 1
    fi

    # consult cache first
    case $tid_tif in
    {*})
        local circ=$(echo "$tid_tif" | sed -n 's/^{\([^}]*\)}.*$/\1/p')
        net_alias_lookup_dev $circ $tid_var TID_ && return 0
        ;;
    *!)
        net_alias_lookup_dev ${tid_tif%!} $tid_var TID_ && return 0
        ;;
    *)
        net_alias_lookup_dev $tid_tif $tid_var TID_ && return 0
        ;;
    esac

    eval $tid_var=
    local hook
    for hook in $translate_net_if_hooks
    do
        $hook "$tid_tif" "$tid_var" && break
    done

    eval local tid_res=\$$tid_var
    if [ -z "$tid_res" ]
    then
        case $tid_tif in
        any|*+)
            tid_res=$tid_tif
        ;;
        *!)
            tid_res=${tid_tif%!}
        ;;
        {*})
            tid_res=$(circuit_get_interface "$tid_tif")
        ;;
        *)
            if [ -d /sys/class/net/$tid_tif ]
            then
                tid_res=$tid_tif
            fi
        ;;
        esac
    fi

    if [ -z "$tid_res" ]
    then
        if [ -z "$tid_ignore" ]
        then
            log_error "translate_net_if: cannot translate interface '$tid_tif'"
        fi
        return 1
    else
        case $tid_res in
        any|*+)
            ;; # don't add alias for "any" and wildcard names
        *)
            case $tid_tif in
            {*})
                net_alias_add $circ $tid_res TID_
                ;;
            *!)
                net_alias_add ${tid_tif%!} $tid_res TID_
                ;;
            *)
                net_alias_add $tid_tif $tid_res TID_
                ;;
            esac
            ;;
        esac
        eval $tid_var=\"$tid_res\"
        return 0
    fi
}

# reverse lookup for interface names, delivers the symbolic name for a device
translate_net_if_reverse ()
{
    net_alias_lookup_name $1 $2 TID_
}

#----------------------------------------------------------------------------
# Splits an address into address and port (if the latter is available).
# The address is then resolved using circuit_resolve_address().
#
# Input:
#   $1 = address to split
#   $2 = name of variable receiving the extracted and resolved address(es)
#   $3 = name of variable receiving the extracted port (if available)
# Exit code:
#    0   = if at least one address has been found
#    !=0 = if no addresses could be detected
#----------------------------------------------------------------------------
split_addr_port_hooks=
split_addr_port()
{
    local sap_addr=$1
    local sap_varaddr=$2
    local sap_varport=$3

    if [ -z "$sap_varaddr" -o -z "$sap_varport" ]
    then
        return 1
    fi

    eval $sap_varaddr= $sap_varport=
    local _hook _addr=
    for _hook in $split_addr_port_hooks
    do
        if $_hook "$sap_addr" _addr "$sap_varport"
        then
            break
        fi
    done

    # if no port has been found, assume we have only an address
    : ${_addr:=$sap_addr}
    _addr=$(circuit_resolve_address "$_addr")
    eval ${sap_varaddr}=\$_addr
    [ -n "$_addr" ]
}

#----------------------------------------------------------------------------
# Returns the MAC address for a given Ethernet interface.
#
# $1     = network interface
# output = MAC address or the empty string if the network interface is
#          unknown or not an Ethernet interface
#----------------------------------------------------------------------------
get_mac_address_from_ethernet_interface()
{
    ip link show dev $1 |
        sed -n 's,^[[:space:]]*link/ether \([^[:space:]]*\).*$,\1,p'
}

networking_api='yes'

# include layer-3 specific extensions
for f in /etc/boot.d/networking.inc.*
do
    . $f
done

fi # $networking_api != yes
