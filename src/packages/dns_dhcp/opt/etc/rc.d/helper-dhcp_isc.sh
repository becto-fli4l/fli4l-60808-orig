#!/bin/sh
#------------------------------------------------------------------------------
# /etc/rc.d/helper-dhcp_isc.sh - configure isc-dhcpd
#
# Creation:     2007-02-03  LanSpezi
# Last Update:  $Id$
#------------------------------------------------------------------------------

create_dhcp_deny_mac ()
{
    if [ "$DHCP_DENY_MAC_N" != "0" ]
    then
        {
            echo ""
            echo "group"
            echo "{"
            echo "    use-host-decl-names on;"
            echo "    default-lease-time $DHCP_LS_TIME_FIX;"
            echo "    max-lease-time $DHCP_MAX_LS_TIME_FIX;"
            echo ""

            [ 0$DHCP_DENY_MAC_N -eq 0 ] || for idx in `seq 1 $DHCP_DENY_MAC_N`
            do
                eval macaddr='$DHCP_DENY_MAC_'$idx
                echo "    host blocked_mac_$idx"
                echo "    {"
                echo "        hardware ethernet $macaddr;"
                echo "        deny booting;"
                echo "    }"
            done
            echo "}"
        } > /etc/dhcpd.d/dhcp_deny_mac.conf
    fi
}

pxe_option()
{
    local var=$1
    local index=$2

    eval local pxe_file="\$${var}_${index}_PXE_FILENAME"

    if [ -z "$pxe_file" ]; then
        return
    fi

    eval local pxe_server_name="\$${var}_${index}_PXE_SERVERNAME"
    eval local pxe_server_ip="\$${var}_${index}_PXE_SERVERIP"
    eval local pxe_options="\$${var}_${index}_PXE_OPTIONS"

    if [ "$pxe_server_name" ]; then
        echo "        next-server $pxe_server_name;"
    else
        [ "$pxe_server_ip" ] && echo "        next-server $pxe_server_ip;"
    fi
    echo "        filename \"$pxe_file\";"
}

create_dhcp_hosts ()
{

    [ 0$IP_NET_N -eq 0 ] || for idn in `seq 1 $IP_NET_N`
    do
    {
        mac_addr_found=false
        eval net='$IP_NET_'$idn
        case "$net" in
            '' | {*}*) continue ;;
        esac

        eval net_dev='$IP_NET_'$idn'_DEV'
        net_ip=`netcalc network $net`
        net_bits=`netcalc netmaskbits $net`

        [ 0$HOST_N -eq 0 ] || for idx in `seq 1 $HOST_N`
        do
            eval macaddr='$HOST_'$idx'_MAC'
            if [ -n "$macaddr" ]
            then
                eval hostname='$HOST_'$idx'_NAME'
                eval ipaddr='$HOST_'$idx'_IP4'

                host_net=`netcalc network $ipaddr/$net_bits`

                if [ "$host_net" = "$net_ip" ]
                then
                     if [ "$mac_addr_found" = "false" ]
                     then
                         # write group-header if first host found
                        echo ""
                        echo "group"
                        echo "{"
                        echo "    use-host-decl-names on;"
                        echo "    default-lease-time $DHCP_LS_TIME_FIX;"
                        echo "    max-lease-time $DHCP_MAX_LS_TIME_FIX;"
                        echo ""
                        mac_addr_found=true
                    fi
                    # write host-infos
                    echo "    host $hostname"
                    echo "    {"
                    echo "        hardware ethernet $macaddr;"
                    echo "        fixed-address $ipaddr;"
                    pxe_option HOST $idx
                    echo "    }"
                fi
            fi
        done
        [ "$mac_addr_found" = "true" ] && echo "}"
    } > /etc/dhcpd.d/dhcp_hosts_NET$idn.conf
    done
    # remove file if no host found!
    [ "$mac_addr_found" = "false" ] && rm /etc/dhcpd.d/dhcp_hosts_NET$idn.conf
}

create_dhcp_range_extra ()
{
    [ 0$DHCP_EXTRA_RANGE_N -eq 0 ] || for idx in `seq 1 $DHCP_EXTRA_RANGE_N`
    do
        {
            eval range_start='$DHCP_EXTRA_RANGE_'$idx'_START'
            eval range_end='$DHCP_EXTRA_RANGE_'$idx'_END'
            eval gw='$DHCP_EXTRA_RANGE_'$idx'_GATEWAY'
            eval netmask='$DHCP_EXTRA_RANGE_'$idx'_NETMASK'
            network=`netcalc network $range_start $netmask`
            broadcast=`netcalc broadcast $network $netmask`

            echo "subnet $network netmask $netmask"
            echo "{"
            if [ -n "$range_start" -a -n "$range_end" ]
            then
                echo "    range $range_start $range_end;"
            fi
            echo "    default-lease-time $DHCP_LS_TIME_DYN;"
            echo "    max-lease-time $DHCP_MAX_LS_TIME_DYN;"
            echo "    option subnet-mask $netmask;"
            echo "    option broadcast-address $broadcast;"
            echo "    option routers $gw;"
            
            # dns-server (option=6)
            dhcpd_server_option DHCP_EXTRA_RANGE DNS $idx
            if [ "$_DNS_server" != "none" ]
            then 
                echo "    option domain-name-servers $_DNS_server;"
            fi

            # wins-server (option=19,44,45,46,47)
            dhcpd_server_option DHCP_EXTRA_RANGE WINS $idx
            if [ "$_WINS_server" != "none" ]
            then 
                echo "    option netbios-name-servers $_WINS_server;"
                echo "    option netbios-dd-server $_WINS_server;"
                echo "    option netbios-node-type 8;"
            else
                echo "    option netbios-node-type 1;"
            fi

            # NTP-Time-Server (option=42)
            dhcpd_server_option DHCP_EXTRA_RANGE NTP $idx
            if [ "$_NTP_server" != "none" ]
            then 
                echo "    option ntp-servers $_NTP_server;"
            fi

            echo "}"
        } > /etc/dhcpd.d/dhcp_range_extra_EXT$idx.conf
    done
}

write_dhcp_range_local ()
{
    idnet=$1
    {
        log_info "create dhcp-config for net defined in IP_NET_$idnet [$2]"
        echo "$net_dev" >> /var/run/dhcpd.iflist
            # group-header for defined range
            echo ""
            echo "subnet $net_ip netmask $net_mask"
            echo "{"

            if [ -n "$range_start" -a -n "$range_end" ]
            then
                echo "    range $range_start $range_end;"
            fi
            echo "    default-lease-time $DHCP_LS_TIME_DYN;"
            echo "    max-lease-time $DHCP_MAX_LS_TIME_DYN;"
            echo "    option subnet-mask $net_mask;"

            eval gateway='$DHCP_RANGE_'$idnet'_GATEWAY'
            case $gateway in
                '') echo "    option routers $ip_local;" ;;
                none) ;;
                *)  echo "    option routers $gateway;" ;;
            esac
            echo "    option broadcast-address $ip_bcast;"

            # dns-server (option=6)
            dhcpd_server_option DHCP_RANGE DNS $idnet
            if [ "$_DNS_server" != "none" ]
            then 
                echo "    option domain-name-servers $_DNS_server;"
            fi

            eval dns_domain='$DHCP_RANGE_'$idnet'_DNS_DOMAIN'
            case x$dns_dnsdomain in
                x)
                    if [ "$DOMAIN_NAME" != "" ]
                    then
                        echo "    option domain-name \"$DOMAIN_NAME\";"
                    fi
                    ;;
                x*)
                    echo "    option domain-name \"$dns_domain\";"
                    ;;
            esac

            # wins-server (option=19,44,45,46,47)
            dhcpd_server_option DHCP_RANGE WINS $idnet
            if [ "$_WINS_server" != "none" ]
            then 
                echo "    option netbios-name-servers $_WINS_server;"
                echo "    option netbios-dd-server $_WINS_server;"
                echo "    option netbios-node-type 8;"
            else
                echo "    option netbios-node-type 1;"
            fi

            # NTP-Time-Server (option=42)
            dhcpd_server_option DHCP_RANGE NTP $idnet
            if [ "$_NTP_server" != "none" ]
            then 
                echo "    option ntp-servers $_NTP_server;"
            fi

            pxe_option DHCP_RANGE $idnet
            echo "}"
    } > /etc/dhcpd.d/dhcp_range_NET$idnet.conf
}

create_dhcp_range_local ()
{
    [ 0$DHCP_RANGE_N -eq 0 ] || for idx in `seq 1 $DHCP_RANGE_N`
    do
        eval range_start='$DHCP_RANGE_'$idx'_START'
        eval range_end='$DHCP_RANGE_'$idx'_END'
        eval net_ref='$DHCP_RANGE_'$idx'_NET'
        eval net='$'$net_ref
        eval net_dev='$'$net_ref'_DEV'

        case "$net" in
            '' | {*}*) continue ;;
        esac
        net_ip=`netcalc network $net`
        net_bits=`netcalc netmaskbits $net`
        dhcp_net=`netcalc network $range_start/$net_bits`
        ip_local=`echo $net | sed -e 's/\/.*//'`
        net_mask=`netcalc netmask $net`
        ip_bcast=`netcalc broadcast $net`
        write_dhcp_range_local $idx "DHCP-Range"
    done

    # Is this really needed?
    # iterate over the networks to write also DHCP-Information for dhcp-host
    # but no range is defined for the network of the host
    [ 0$IP_NET_N -eq 0 ] || for idn in `seq 1 $IP_NET_N`
    do
        range_start=
        range_end=
        eval net='$IP_NET_'$idn
        case "$net" in
            '' | {*}*) continue ;;
        esac
        ip_local=`echo $net | sed -e 's/\/.*//'`
        eval net_dev='$IP_NET_'$idn'_DEV'
        net_ip=`netcalc network $net`
        net_bits=`netcalc netmaskbits $net`
        net_mask=`netcalc netmask $net`
        ip_bcast=`netcalc broadcast $net`
        if [ -f /etc/dhcpd.d/dhcp_range_NET$idn.conf ]
        then
            continue
        fi
        if [ -f /etc/dhcpd.d/dhcp_hosts_NET$idn.conf ]
        then
            #write_dhcp_range_local $idn "only DHCP-Hosts"
            continue
        fi
    done

    {
        # global parameters
        echo "deny bootp;"
        echo "option domain-name \"$DOMAIN_NAME\";"
        echo "ignore client-updates;"
        echo "ping-check true;"
        echo "authoritative;"

        # include-statement works not with dhcpd 2.0......
        # ------------------------------------------------
        #for dhcpfile in /etc/dhcpd.d/*
        #do
        #    echo "include \"$dhcpfile\""
        #done

        for i in /etc/dhcpd.d/dhcp_range_NET*.conf \
            /etc/dhcpd.d/dhcp_range_extra_*.conf   \
            /etc/dhcpd.d/dhcp_deny_mac.conf        \
            /etc/dhcpd.d/dhcp_hosts_*.conf; do
          [ -f $i ] && cat $i
        done
    } > /etc/dhcpd.conf

}

dhcp_if_list=

dhcp_add_rule ()
{
    case x$dhcp_if_list in
        x)
            fw_add_chain filter in-dhcpd
            fw_prepend_rule filter INPUT-tail "prot:udp 67 in-dhcpd" "dhcp requests access"
            fw_append_rule filter in-dhcpd DROP
            ;;
    esac
    case $dhcp_if_list in
        *$1*)
            # ignore it, its already there
            ;;
        *)
            fw_prepend_rule filter in-dhcpd "if:$1:any ACCEPT"  "accept dhcp requests from net connected to device $1"
            dhcp_if_list="$dhcp_if_list $1"
            ;;
    esac
}

conf_firewall_dhcp ()
{
    # add rule in "in-dhcp" for any interface with defined dhcp-ranges
    while read ifname
    do
        case $iflist in
            *$ifname*)
                # ignore it, its already there
                ;;
            *)
                dhcp_add_rule $ifname
                ;;
        esac
    done < /var/run/dhcpd.iflist
}

