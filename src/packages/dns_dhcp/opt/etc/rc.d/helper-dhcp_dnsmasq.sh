#!/bin/sh
#------------------------------------------------------------------------------
# /etc/rc.d/helper-dhcp_dnsmasq.sh - configure dnsmasq dhcp
#
# Creation:     2007-02-03  LanSpezi
# Last Update:  $Id$
#------------------------------------------------------------------------------

pxe_option()
{
    #            $1         $2       $3          $4
    # pxe_option DHCP_RANGE $idrange NET$idnet
    # pxe_option HOST       $idx     pxehost$idx $macaddr
    var=$1
    index=$2
    label=$3
    mac=$4

    eval pxe_file="\$${var}_${index}_PXE_FILENAME"

    if [ -z "$pxe_file" ]
    then
        return
    fi

    eval pxe_server_name="\$${var}_${index}_PXE_SERVERNAME"
    eval pxe_server_ip="\$${var}_${index}_PXE_SERVERIP"
    eval pxe_options="\$${var}_${index}_PXE_OPTIONS"

    if [ "$label" ]
    then
        blabel="net:${label},"
        olabel="${label},"
    fi

    if [ "$mac" ]
    then
        echo "dhcp-mac=$label,$mac"
    fi

    case x$pxe_server_name in
        x) echo "dhcp-boot=${blabel}${pxe_file}" ;;
        *)
            case x$pxe_server_ip in
                x) echo "dhcp-boot=${blabel}${pxe_file},${pxe_server_name}" ;;
                *) echo "dhcp-boot=${blabel}${pxe_file},${pxe_server_name},${pxe_server_ip}" ;;
            esac
    esac
    [ -z "$pxe_options" ] || for opt in $pxe_options; do
        echo "dhcp-option=${olabel}$opt"
    done
}

write_dhcp_range_local ()
{
    idnet=$1
    idrange=$2
    echo "$idnet" >> /var/run/dhcp_range_net.list_tmp
    {
        log_info "create dhcp-config for net defined in IP_NET_$idnet [$3]"
        if [ -n "$range_start" -a -n "$range_end" ]
        then
            echo "dhcp-range=NET$idnet,$range_start,$range_end,$DHCP_LS_TIME_DYN"
        else
            # if no range define, activate only static dhcp for the network
            echo "dhcp-range=NET$idnet,$net_ip,static"
        fi
        # subnet-mask (option=1)
        echo "dhcp-option=NET$idnet,1,$net_mask"

        # default-router (option=3)
        eval gateway='$DHCP_RANGE_'$idrange'_GATEWAY'
        case x$gateway in
            x)
                echo "dhcp-option=NET$idnet,3,$ip_local"
                ;;
            xnone)
                echo "dhcp-option=NET$idnet,3"
                ;;
            x*)
                echo "dhcp-option=NET$idnet,3,$gateway"
               ;;
        esac

        # broadcast address (option=28)
        echo "dhcp-option=NET$idnet,28,$ip_bcast"

        # dns-server (option=6)
        dhcpd_server_option DHCP_RANGE DNS $idrange
        if [ "$_DNS_server" != "none" ]
        then 
            echo "dhcp-option=NET$idnet,6,$_DNS_server"
        fi

        # dns-domain (option=15)
        eval dns_domain='$DHCP_RANGE_'$idrange'_DNS_DOMAIN'
        case x$dns_domain in
            x)
                if [ "$DOMAIN_NAME" != "" ]
                then
                    echo "dhcp-option=NET$idnet,15,$DOMAIN_NAME"
                fi
                ;;
            x*)
                echo "dhcp-option=NET$idnet,15,$dns_domain"
                echo "domain=$dns_domain,$net"
                ;;
        esac

        # Set the "all subnets are local" flag
        echo "dhcp-option=NET$idnet,27,1"

        # wins-server (option=19,44,45,46,47)
        dhcpd_server_option DHCP_RANGE WINS $idrange
        if [ "$_WINS_server" != "none" ]
        then 
            # option ip-forwarding off
            echo "dhcp-option=NET$idnet,19,0"
            # set netbios-over-TCP/IP-nameserver(s) aka WINS server(s)
            echo "dhcp-option=NET$idnet,44,$_WINS_server"
            # netbios datagram distribution server
            echo "dhcp-option=NET$idnet,45,$_WINS_server"
            # netbios node type
            echo "dhcp-option=NET$idnet,46,8"
        else
            # netbios node type
            echo "dhcp-option=NET$idnet,46,1"
        fi
                
        # NTP-Time-Server (option=42)
        dhcpd_server_option DHCP_RANGE NTP $idrange
        if [ "$_NTP_server" != "none" ]
        then 
             echo "dhcp-option=NET$idnet,42,$_NTP_server"
        fi
        
        # MTU (option=26)
        eval mtu='$DHCP_RANGE_'$idrange'_MTU'
        [ -n "$mtu" ] && echo "dhcp-option=NET$idnet,26,$mtu"

        # PXE configuration
        pxe_option DHCP_RANGE $idrange NET$idnet

        echo ""
        # WPAD-URL (option=252"
        eval optwpad=\$DHCP_RANGE_${idrange}_OPTION_WPAD
        if [ "$optwpad" = "yes" ]
        then
            eval wpadurl=\$DHCP_RANGE_${idrange}_OPTION_WPAD_URL
            if [ "x$wpadurl" = "x" ]
            then
                echo "dhcp-option=NET$idnet,252,\"\\n\""
            else
                echo "dhcp-option=NET$idnet,252,${wpadurl}"
            fi
        fi
        echo ""

        # User defined extra options
        # DHCP_RANGE_x_OPTION_N and DHCP_RANGE_x_OPTION_y
        eval drion=\$DHCP_RANGE_${idrange}_OPTION_N
        if [ x$drion != "x" ]
        then
            [ 0$drion -eq 0 ] || for optionn in `seq 1 $drion`
            do
                eval option_val=\$DHCP_RANGE_${idrange}_OPTION_${optionn}
                echo "dhcp-option=NET$idnet,$option_val"
            done
        fi
    } > /etc/dnsmasq.d/dhcp_range_NET$idnet.conf
}

create_dhcp_range_local ()
{
    # write dhcp-config for all defined ranges
    [ 0$DHCP_RANGE_N -eq 0 ] || for idx in `seq 1 $DHCP_RANGE_N`
    do
        eval range_start='$DHCP_RANGE_'$idx'_START'
        case $range_start in
            '') continue ;;
        esac

        eval range_end='$DHCP_RANGE_'$idx'_END'

        eval net_ref_by_dhcp=$`echo $DHCP_RANGE_$idx_NET`

        [ 0$IP_NET_N -eq 0 ] || for idn in `seq 1 $IP_NET_N`
        do
            eval net='$IP_NET_'$idn
            case "$net" in
                '' | {*}*) continue ;;
            esac
            net_ip=`netcalc network $net`
            net_bits=`netcalc netmaskbits $net`
            dhcp_net=`netcalc network $range_start/$net_bits`
            if [ "$dhcp_net" = "$net_ip" ]
            then
                ip_local=`echo $net | sed -e 's/\/.*//'`
                eval net_dev='$IP_NET_'$idn'_DEV'
                net_mask=`netcalc netmask $net`
                ip_bcast=`netcalc broadcast $net`
                write_dhcp_range_local $idn $idx "DHCP-Range"
                break
            fi
        done
    done

    # iterate over the networks to write also DHCP-Information for dhcp-host
    # but no range is defined for the network of the host
    range_start=
    range_end=
    [ 0$IP_NET_N -eq 0 ] || for idn in `seq 1 $IP_NET_N`
    do
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
        if [ -f /etc/dnsmasq.d/dhcp_range_NET$idn.conf ]
        then
            continue
        fi
        if [ -f /etc/dnsmasq.d/dhcp_hosts_NET$idn.conf ]
        then
            for idx in `seq 1 $DHCP_RANGE_N`
            do
                eval net=\$DHCP_RANGE_${idx}_NET
                if [ "$net" = "IP_NET_$idn" ]
                then
                    write_dhcp_range_local $idn $idx "only DHCP-Hosts (extra DHCP-Options)"
                    break
                fi
            done
            if [ ! -f /etc/dnsmasq.d/dhcp_range_NET$idn.conf ]
            then
                write_dhcp_range_local $idn 0 "only DHCP-Host"
            fi
        fi
    done

    # Sort List of dhcp-Networks (used by status_dhcp.cgi)
    if [ -f /var/run/dhcp_range_net.list_tmp ]
    then
        sort -n -u /var/run/dhcp_range_net.list_tmp > /var/run/dhcp_range_net.list
       rm /var/run/dhcp_range_net.list_tmp
    fi
}


create_dhcp_range_extra ()
{
    # write dhcp-config for all extra ranges
    [ 0$DHCP_EXTRA_RANGE_N -eq 0 ] || for idx in `seq 1 $DHCP_EXTRA_RANGE_N`
    do
        {
            eval range_start='$DHCP_EXTRA_RANGE_'$idx'_START'
            eval range_end='$DHCP_EXTRA_RANGE_'$idx'_END'
            eval range_netmask='$DHCP_EXTRA_RANGE_'$idx'_NETMASK'
            if [ -n "$range_start" -a -n "$range_end" ]
            then
                echo "dhcp-range=EXT$idx,$range_start,$range_end,$range_netmask,$DHCP_LS_TIME_DYN"
            fi

            #subnet-mask (option=1)
            echo "dhcp-option=EXT$idx,1,$range_netmask"

            # default-router (option=3)
            eval ip_gateway='$DHCP_EXTRA_RANGE_'$idx'_GATEWAY'
            echo "dhcp-option=EXT$idx,3,$ip_gateway"

            # broadcast-address (option=28)
            ip_bcast=`netcalc broadcast $range_start $range_netmask`
            echo "dhcp-option=EXT$idx,28,$ip_bcast"

            # dns-server (option=6)
            dhcpd_server_option DHCP_EXTRA_RANGE DNS $idx
            if [ "$_DNS_server" != "none" ]
            then 
                echo "dhcp-option=EXT$idx,6,$_DNS_server"
            else
                echo "dhcp-option=EXT$idx,6"
            fi
            
            # Set the "all subnets are local" flag
            echo "dhcp-option=EXT$idx,72,1"

            # wins-server (option=19,44,45,46,47)
            dhcpd_server_option DHCP_EXTRA_RANGE WINS $idx
            if [ "$_WINS_server" != "none" ]
            then 
                # option ip-forwarding off
                echo "dhcp-option=EXT$idx,19,0"
                # set netbios-over-TCP/IP-nameserver(s) aka WINS server(s)
                echo "dhcp-option=EXT$idx,44,$_WINS_server"
                # netbios datagram distribution server
                echo "dhcp-option=EXT$idx,45,$_WINS_server"
                # netbios node type
                echo "dhcp-option=EXT$idx,46,8"
            else
                # netbios node type
                echo "dhcp-option=EXT$idx,46,1"
            fi
                    
            # NTP-Time-Server (option=42)
            dhcpd_server_option DHCP_EXTRA_RANGE $idx EXTRA
            if [ "$_NTP_server" != "none" ]
            then 
                 echo "dhcp-option=EXT$idx,42,$_NTP_server"
            else
                 echo "dhcp-option=EXT$idx,42"
            fi


            # MTU (option=26)
            eval mtu='$DHCP_EXTRA_RANGE_'$idx'_MTU'
            [ -n "$mtu" ] && echo "dhcp-option=EXT$idx,26,$mtu"

        } > /etc/dnsmasq.d/dhcp_range_extra_EXT$idx.conf
    done
}

create_dhcp_hosts ()
{
    if [ -f /etc/dnsmasq.d/dhcp_hosts_*.conf ]
    then
        rm /etc/dnsmasq.d/dhcp_hosts_*.conf
    fi

    [ 0$HOST_N -eq 0 ] || for idx in `seq 1 $HOST_N`
    do
        eval macaddr='$HOST_'$idx'_MAC'
        eval macaddr2='$HOST_'$idx'_MAC2'
        eval dhcp_typ='$HOST_'$idx'_DHCPTYP'
        if [ "$macaddr" != ""  -o  "$dhcp_typ" = "hostname" ]
        then
            eval ipaddr='$HOST_'$idx'_IP4'
            eval host='$HOST_'$idx'_NAME'

            [ 0$IP_NET_N -eq 0 ] || for idn in `seq 1 $IP_NET_N`
            do
                eval net='$IP_NET_'$idn
                case "$net" in
                    '' | {*}*) continue ;;
                esac
                eval net_dev='$IP_NET_'$idn'_DEV'
                net_ip=`netcalc network $net`
                net_bits=`netcalc netmaskbits $net`
                host_net=`netcalc network $ipaddr/$net_bits`
                if [ "$host_net" = "$net_ip" ]
                then
                    # host defined in one IP_NET_x
                    if [ "$dhcp_typ" = "hostname" ]
                    then
                        echo "dhcp-host=$host,$ipaddr,$host,$DHCP_LS_TIME_FIX,net:NET$idn" >> /etc/dnsmasq.d/dhcp_hosts_NET$idn.conf
                    else
                        if [ "$macaddr2" = "" ]
                        then
                            echo "dhcp-host=$macaddr,$ipaddr,$host,$DHCP_LS_TIME_FIX,net:NET$idn" >> /etc/dnsmasq.d/dhcp_hosts_NET$idn.conf
                        else
                            echo "dhcp-host=$macaddr,$macaddr2,$ipaddr,$host,$DHCP_LS_TIME_FIX,net:NET$idn" >> /etc/dnsmasq.d/dhcp_hosts_NET$idn.conf
                        fi
                    fi
                fi
            done

            [ 0$DHCP_EXTRA_RANGE_N -eq 0 ] || for idn in `seq 1 $DHCP_EXTRA_RANGE_N`
            do
                eval net='$DHCP_EXTRA_RANGE_'$idn'_START'
                eval net_dev='$DHCP_EXTRA_RANGE_'$idn'_DEVICE'
                eval net_mask='$DHCP_EXTRA_RANGE_'$idn'_NETMASK'
                net_bits=`netcalc netmaskbits $net_mask`
                net_ip=`netcalc network $net/$net_bits`
                host_net=`netcalc network $ipaddr/$net_bits`
                if [ "$host_net" = "$net_ip" ]
                then
                    # host defined in one DHCP_EXTRA_RANGE
                    if [ "$dhcp_typ" = "hostname" ]
                    then
                        echo "dhcp-host=$host,$ipaddr,$host,$DHCP_LS_TIME_FIX,net:NET$idn" >> /etc/dnsmasq.d/dhcp_hosts_EXT$idn.conf
                    else
                        if [ "$macaddr2" = "" ]
                        then
                            echo "dhcp-host=$macaddr,$ipaddr,$host,$DHCP_LS_TIME_FIX,net:NET$idn" >> /etc/dnsmasq.d/dhcp_hosts_EXT$idn.conf
                        else
                            echo "dhcp-host=$macaddr,$macaddr2,$ipaddr,$host,$DHCP_LS_TIME_FIX,net:NET$idn" >> /etc/dnsmasq.d/dhcp_hosts_EXT$idn.conf
                        fi
                    fi
                fi
            done
        fi

        if [ "$macaddr" != ""  -a  "$dhcp_typ" = "mac" ]
        then
            pxe_option HOST $idx pxehost$idx $macaddr >> /etc/dnsmasq.d/dhcp_pxehosts.conf
        fi
    done
}

create_dhcp_deny_mac ()
{
    {
        echo "### DHCP_DENY_MAC ###"
        [ 0$DHCP_DENY_MAC_N -eq 0 ] || for idx in `seq 1 $DHCP_DENY_MAC_N`
        do
            eval macaddr='$DHCP_DENY_MAC_'$idx
            echo "dhcp-host=$macaddr,ignore"
        done
    } > /etc/dnsmasq.d/dhcp_deny_mac.conf
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
            fw_prepend_rule filter in-dhcpd "if:$1:any ACCEPT" "accept dhcp requests from net connected to $1"
            dhcp_if_list="$dhcp_if_list $1"
            ;;
    esac
}

conf_firewall_dhcp ()
{
    # add rule in "in-dhcp" for any interface with defined dhcp-ranges
    [ 0$IP_NET_N -eq 0 ] || for idx in `seq 1 $IP_NET_N`
    do
        if [ -f /etc/dnsmasq.d/dhcp_range_NET$idx.conf -o \
             -f /etc/dnsmasq.d/dhcp_range_extra_EXT$idx.conf ]
        then
            eval range_dev='IP_NET_'$idx'_DEV'
            dhcp_add_rule $range_dev
        fi
    done
}
