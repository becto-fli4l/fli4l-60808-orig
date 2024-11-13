#!/bin/sh
#------------------------------------------------------------------------------
# /etc/rc.d/helper-dns_dnsmasq.sh - configure dnsmasq dns
#
# Creation:     2007-02-03  LanSpezi
# Last Update:  $Id$
#------------------------------------------------------------------------------

create_resolv_dnsserver ()
{
    # create /etc/resolv.dnsmasq
    # forwarder(s) for dnsmasq - changed by ip-up.dnsmasq if USEPEERDNS
    {
        echo "# forwarders for dnsmasq"
        local forwarder addresses addr port
        for forwarder in $DNS_FORWARDERS
        do
            split_addr_port "$forwarder" addresses port || continue
            for addr in $addresses
            do
                echo "nameserver $addr"
            done
        done
    } > /etc/resolv.dnsmasq
    chown dns:dns /etc/resolv.dnsmasq
    chmod 600 /etc/resolv.dnsmasq
}

create_resolv_dnsserver_n ()
{
    # create /etc/dnsmasq.d/dns_resolver.conf
    # forwarder(s) with optional port for dnsmasq
    # can be changed by ip-up.dnsmasq if USEPEERDNS
    {
        echo "# forwarders for dnsmasq"
        local forwarder addresses addr port
        for forwarder in $DNS_FORWARDERS
        do
            split_addr_port "$forwarder" addresses port || continue
            for addr in $addresses
            do
                echo "server=$addr#$port"
            done
        done
    } > /etc/dnsmasq.d/dns_resolver.conf

    # always create /etc/resolv.dnsmasq to ease USEPEERDNS handling
    > /etc/resolv.dnsmasq
    chown dns:dns /etc/resolv.dnsmasq
    chmod 600 /etc/resolv.dnsmasq
}

create_dns_forbidden ()
{
    #
    # create /etc/dnsmasq.d/dns_forbidden.conf
    #
    for idx in `seq 1 $DNS_FORBIDDEN_N`
    do
        eval dom='$DNS_FORBIDDEN_'$idx
        echo "local=/$dom/" >> /etc/dnsmasq.d/dns_forbidden.conf
    done
}

create_dns_redirect ()
{
    #
    # create /etc/dnsmasq.d/dns_redirect.conf
    #
    for idx in `seq 1 $DNS_REDIRECT_N`
    do
        eval dom='$DNS_REDIRECT_'$idx
        eval domip='$DNS_REDIRECT_'$idx'_IP'
        echo "address=/$dom/$domip" >> /etc/dnsmasq.d/dns_redirect.conf
    done
}

dnsforwarders_with_port()
{
    local forwarder addresses port
    for forwarder in $DNS_FORWARDERS
    do
        split_addr_port "$forwarder" addresses port
        [ -n "$port" ] && return 0
    done
    return 1
}

create_dns_config ()
{
    # create /etc/dnsmasq.conf
    {
        echo "user=dns"
        echo "group=dns"
        if dnsforwarders_with_port
        then
            create_resolv_dnsserver_n
        else
            create_resolv_dnsserver
        fi
        echo "resolv-file=/etc/resolv.dnsmasq"
        echo "no-poll"
        echo "no-negcache"
        echo "log-facility=local0"

        if [ "$DNS_VERBOSE" = "yes" ]
        then
            echo "log-queries"
        fi

        # Listen-IP's (Interfaces)
        if [ 0"$DNS_LISTEN_N" -ne 0 ]
        then
            if [ "$DNS_BIND_INTERFACES" = "yes" ]
            then
                echo "bind-interfaces"
            fi
            echo "listen-address=127.0.0.1"
            for idx in `seq 1 $DNS_LISTEN_N`
            do
                eval ipaddr='$DNS_LISTEN_'$idx
                case $ipaddr in
                IP_NET_*|*.*.*.*)
                    prot=4
                    translate_ip_net $ipaddr DNS_LISTEN_$idx
                    ;;
                IPV6_NET_*|*:*)
                    prot=6
                    translate_ip6_net $ipaddr DNS_LISTEN_$idx
                    ;;
                esac
                if [ $? -eq 0 ]
                then
                    local addr
                    for addr in $res
                    do
                        if ip -$prot addr show | grep -q "$addr/"
                        then
                            echo "listen-address=$addr"
                        else
                            log_error "DNS_LISTEN_$idx='$ipaddr' ($addr) is not a local IP"
                        fi
                    done
                fi
            done
        fi

        # Authoritative IP's (Interfaces)
        if [ "$DNS_AUTHORITATIVE" = yes ]
        then
            res=
            case $DNS_AUTHORITATIVE_LISTEN in
            IP_NET_*_IPADDR)
                translate_ip_net $DNS_AUTHORITATIVE_LISTEN DNS_AUTHORITATIVE_LISTEN
                ;;
            IPV6_NET_*_IPADDR)
                translate_ip6_net $DNS_AUTHORITATIVE_LISTEN DNS_AUTHORITATIVE_LISTEN
                ;;
            *)
                translate_net_if $DNS_AUTHORITATIVE_LISTEN res
                ;;
            esac

            if [ -n "$res" ]
            then
                list_foreach 'echo "auth-server=$DNS_AUTHORITATIVE_NS,$1"' $res
            fi
        fi

        # Zone configuration
        if [ 0"$DNS_ZONE_NETWORK_N" -ne 0 ]
        then
            zone_networks=
            sep=
            for idx in `seq 1 $DNS_ZONE_NETWORK_N`
            do
                eval zone_network=\$DNS_ZONE_NETWORK_$idx
                zone_network=$(circuit_resolve_address "$zone_network")
                zone_networks="$zone_networks$sep$zone_network"
                sep=,
            done
            echo "auth-zone=$DOMAIN_NAME,$zone_networks"

            if [ "$DNS_MX_SERVER" != "" ]
            then
                # according to the man page, mx-host is necessary for
                # authoritative mode (mx-target alone is not sufficient)
                echo "mx-host=$DOMAIN_NAME"
            fi
        fi

        echo "domain-suffix=$DOMAIN_NAME"
        echo "expand-hosts"

        # Reject (and log) addresses from upstream nameservers which are in the private IP ranges.
        #This blocks an attack where a browser behind a firewall is used to probe machines on the local network.
        echo "stop-dns-rebind"
        # Allow loopback addresses to be returned by external DNS servers
        # (cf. ticket #222)
        echo "rebind-localhost-ok"

        if [ -f /etc/hosts.d/hosts.dns ]
        then
            echo "addn-hosts=/etc/hosts.d/hosts.dns"
        fi
        if [ -f /etc/hosts.d/hosts.extra ]
        then
            echo "addn-hosts=/etc/hosts.d/hosts.extra"
        fi

        if [ -f /etc/hosts.global ]
        then
            mv /etc/hosts.global /etc/hosts.d/hosts.global
            echo "addn-hosts=/etc/hosts.d/hosts.global"
        fi

        # file for hosts added dynamically by the webinterface
        echo "# file for hosts added dynamically by the webinterface" > /etc/hosts.d/hosts.dhcp
        echo "#" >> /etc/hosts.d/hosts.dhcp
        echo "addn-hosts=/etc/hosts.d/hosts.dhcp"

        chown -h dns:dns /etc/hosts.d/*
        chmod 666 /etc/hosts.d/* 2>/dev/null

        if [ "$DNS_MX_SERVER" != "" ]
        then
            echo "localmx"
            echo "mx-target=$DNS_MX_SERVER"
        fi

        # forbidden Domains
        create_dns_forbidden

        # redirect Domains
        create_dns_redirect

        if [ "$DNS_FILTERWIN2K" = "yes" ]
        then
            echo "filterwin2k"
        fi

        # set TTL for local hosts. local hosts the ones from /etc/hosts and every DHCP host
        : ${DNS_LOCAL_HOST_CACHE_TTL:=0}
        echo "local-ttl=$DNS_LOCAL_HOST_CACHE_TTL"

        if [ "$DNS_FORWARD_LOCAL" = "no" ]
        then
            echo "domain-needed"
            # do not forward querys for local-Domain
            echo "local=/$DOMAIN_NAME/"
        fi

        echo "min-port=4096"

        mkdir -p /etc/dnsmasq.d

        # include config-files with special-DNS-configs
        echo "conf-dir=/etc/dnsmasq.d"

    } > /etc/dnsmasq.conf
    chown dns:dns /etc/dnsmasq.conf
    chmod 600 /etc/dnsmasq.conf

    # new version of dns_delegate.conf
    {
        cat << EOF
#----------------------------------------------------------------------------
# /etc/dnsmasq.d/dns_delegate.conf
#----------------------------------------------------------------------------
EOF
        [ 0$DNS_ZONE_DELEGATION_N -eq 0 ] || for i in `seq 1 $DNS_ZONE_DELEGATION_N`
        do
            eval upstream_domain_count='$DNS_ZONE_DELEGATION_'$i'_DOMAIN_N'
            [ 0$upstream_domain_count -eq 0 ] || for x in `seq 1 $upstream_domain_count`
            do
                eval upstream_domain='$DNS_ZONE_DELEGATION_'$i'_DOMAIN_'$x
                eval upstream_server_count='$DNS_ZONE_DELEGATION_'$i'_UPSTREAM_SERVER_N'
                [ 0$upstream_server_count -eq 0 ] || for j in `seq 1 $upstream_server_count`
                do
                    eval upstream_ipport='$DNS_ZONE_DELEGATION_'$i'_UPSTREAM_SERVER_'$j'_IP'
                    # split ip/net and port
                    set `echo $upstream_ipport | sed 's/:/ /'`
                    addr=$1
                    port=$2
                    if translate_ip_net $addr 'DNS_ZONE_DELEGATION_'$i'_UPSTREAM_SERVER_'$j'_IP'
                    then
                        upstream_ipport="$res"
                        if [ -n "$port" ]
                        then
                            upstream_ipport="$upstream_ipport#$port"
                        fi
                        eval upstream_query_ipport='$DNS_ZONE_DELEGATION_'$i'_UPSTREAM_SERVER_'$j'_QUERYSOURCEIP'
                        if [ "x$upstream_query_ipport" != "x" ]
                        then
                            # split ip/net and port
                            case $upstream_query_ipport in
                            \[*\]:*)
                                addr=${upstream_query_ipport%:*}
                                addr=${addr#\[}
                                addr=${addr%\]}
                                port=${upstream_query_ipport##*:}
                                ;;
                            *:*)
                                addr=${upstream_query_ipport%:*}
                                port=${upstream_query_ipport##*:}
                                ;;
                            *)
                                addr=$upstream_query_ipport
                                port=
                                ;;
                            esac

                            if translate_ip_net $addr 'DNS_ZONE_DELEGATION_'$i'_UPSTREAM_SERVER_'$j'_QUERYSOURCEIP'
                            then
                                upstream_query_ipport="$res"
                                if [ -n "$port" ]
                                then
                                    upstream_query_ipport="$upstream_query_ipport#$port"
                                fi
                                echo "server=/$upstream_domain/$upstream_ipport@$upstream_query_ipport"
                            else
                                log_error "$upstream_query_ipport from DNS_ZONE_DELEGATION_$i_UPSTREAM_SERVER_$j_QUERYSOURCEIP will be ignored!"
                            fi
                        else
                            echo "server=/$upstream_domain/$upstream_ipport"
                        fi
                    else
                        log_error "$upstream_ipport from DNS_ZONE_DELEGATION_$i_UPSTREAM_SERVER_$j_IP will be ignored!"
                    fi
                done
                echo "rebind-domain-ok=$upstream_domain"
            done
            eval upstream_network_count='$DNS_ZONE_DELEGATION_'$i'_NETWORK_N'
            [ 0$upstream_network_count -eq 0 ] || for x in `seq 1 $upstream_network_count`
            do
                eval upstream_network='$DNS_ZONE_DELEGATION_'$i'_NETWORK_'$x
                upstream_network=$(circuit_resolve_address "$upstream_network")
                if check_rdns $upstream_network
                then
                    [ 0$upstream_server_count -eq 0 ] || for j in `seq 1 $upstream_server_count`
                    do
                        eval upstream_ipport='$DNS_ZONE_DELEGATION_'$i'_UPSTREAM_SERVER_'$j'_IP'
                        # split ip/net and port
                        set `echo $upstream_ipport | sed 's/:/ /'`
                        addr=$1
                        port=$2
                        if translate_ip_net $addr 'DNS_ZONE_DELEGATION_'$i'_UPSTREAM_SERVER_'$j'_IP'
                        then
                            upstream_ipport="$res"
                            if [ -n "$port" ]
                            then
                                upstream_ipport="$upstream_ipport#$port"
                            fi
                            eval upstream_query_ipport='$DNS_ZONE_DELEGATION_'$i'_UPSTREAM_SERVER_'$j'_QUERYSOURCEIP'
                            if [ "x$upstream_query_ipport" != "x" ]
                            then
                                # split ip/net and port
                                set `echo $upstream_query_ipport | sed 's/:/ /'`
                                addr=$1
                                port=$2
                                if translate_ip_net $addr 'DNS_ZONE_DELEGATION_'$i'_UPSTREAM_SERVER_'$j'_QUERYSOURCEIP'
                                then
                                    upstream_query_ipport="$res"
                                    if [ -n "$port" ]
                                    then
                                        upstream_query_ipport="$upstream_query_ipport#$port"
                                    fi
                                    for dnsrev in $(netcalc dnsrev $upstream_network)
                                    do
                                        echo "server=/$dnsrev/$upstream_ipport@$upstream_query_ipport"
                                    done
                                else
                                    log_error "$upstream_query_ipport from DNS_ZONE_DELEGATION_$i_UPSTREAM_SERVER_$j_QUERYSOURCEIP will be ignored!"
                                fi
                            else
                                for dnsrev in $(netcalc dnsrev $upstream_network)
                                do
                                    echo "server=/$dnsrev/$upstream_ipport"
                                done
                            fi
                        else
                            log_error "$upstream_ipport from DNS_ZONE_DELEGATION_$i_UPSTREAM_SERVER_$j_IP will be ignored!"
                        fi
                    done
                else
                    log_error "$upstream_network from DNS_ZONE_DELEGATION_$i_NETWORK_N will be ignored!"
                fi
            done
        done

        [ 0$DNS_REBINDOK_N -eq 0 ] || for i in `seq 1 $DNS_REBINDOK_N`
        do
            eval sp_domain='$DNS_REBINDOK_'$i'_DOMAIN'
            if [ "x$sp_domain" != "x" ]
            then
                echo "rebind-domain-ok=$sp_domain"
            fi
        done

    } > /etc/dnsmasq.d/dns_delegate.conf

    # do not forward reverse-request for local loopback-adresses
    {
        echo ""
        echo "local=/.127.in-addr.arpa/"
        echo ""
    } >> /etc/dnsmasq.d/dns_delegate.conf

    if [ "$DNS_BOGUS_PRIV" = "yes" ]
    then
        {
            cat << EOF

local=/.254.169.in-addr.arpa/
local=/.0.0.192.in-addr.arpa/
local=/.2.0.192.in-addr.arpa/
local=/.100.51.198.in-addr.arpa/
local=/.113.0.203.in-addr.arpa/
EOF
        } >> /etc/dnsmasq.d/dns_delegate.conf
    fi

    # Zone configuration
    local dnsrev
    for idx in $(seq 1 ${DNS_ZONE_NETWORK_N:-0})
    do
        eval zone_network=\$DNS_ZONE_NETWORK_$idx
        zone_network=$(circuit_resolve_address "$zone_network")
        if [ -n "$zone_network" ]
        then
            for dnsrev in $(netcalc dnsrev $zone_network)
            do
                echo "local=/$dnsrev/" >> /etc/dnsmasq.d/dns_delegate.conf
            done
        fi
    done

    # create ipset parameters for dnsmasq
    cat /var/run/ipset.list | while read hostexpr ipsetname
    do
        echo "ipset=/$hostexpr/${ipsetname}4,${ipsetname}6" >> /etc/dnsmasq.d/dns_ipset.conf
    done

    dns_create_private_subnets
}

check_rdns ()
{
    case x$1 in
        x) ;;
        x-REMOVE)
            if [ -f /tmp/dnsmasq_rdns.check ]
            then
                rm /tmp/dnsmasq_rdns.check
            fi
            ;;
        x*)
            eval checknet=`netcalc network $1`

            if [ -f /tmp/dnsmasq_rdns.check ] && grep -q $checknet /tmp/dnsmasq_rdns.check
            then
                log_error "You use $checknet more than once for reverse dns delegation - please check your config!"
                return 1
            else
                echo "$checknet" >> /tmp/dnsmasq_rdns.check
                return 0
            fi
            ;;
    esac
}

# Setups the IPv4 firewall chain controlling incoming DNS requests.
# Input:
#   $1 = default policy (ACCEPT or DROP)
dns_setup_chain()
{
    fw_add_chain filter in-dns-udp
    fw_add_chain filter in-dns-tcp
    fw_append_rule filter INPUT-head "prot:udp 53 in-dns-udp" "filters DNS UDP requests"
    fw_append_rule filter INPUT-head "prot:tcp 53 in-dns-tcp" "filters DNS TCP requests"
    if [ "$OPT_OPENVPN" = "yes" ]
    then
        fw_insert_rule filter in-dns-udp 1 "if:tun+:any any any RETURN" "return UDP DNS requests from VPN device"
        fw_insert_rule filter in-dns-tcp 1 "if:tun+:any any any RETURN" "return TCP DNS requests from VPN device"
    fi
    fw_append_rule filter in-dns-udp $1
    fw_append_rule filter in-dns-tcp $1
}

# Setups the IPv6 firewall chain controlling incoming DNS requests.
# Input:
#   $1 = default policy (ACCEPT or DROP)
dns_setup_chain6()
{
    fw_add_chain6 filter in-dns-udp
    fw_add_chain6 filter in-dns-tcp
    fw_append_rule6 filter INPUT-head "prot:udp 53 in-dns-udp" "filters DNS UDP requests"
    fw_append_rule6 filter INPUT-head "prot:tcp 53 in-dns-tcp" "filters DNS TCP requests"
    if [ "$OPT_OPENVPN" = "yes" ]
    then
        fw_insert_rule6 filter in-dns-udp 1 "if:tun+:any any any RETURN" "return UDP DNS requests from VPN device"
        fw_insert_rule6 filter in-dns-tcp 1 "if:tun+:any any any RETURN" "return TCP DNS requests from VPN device"
    fi
    fw_append_rule6 filter in-dns-udp $1
    fw_append_rule6 filter in-dns-tcp $1
}

conf_firewall_dns4()
{
    if [ 0$DNS_LISTEN_N -ne 0 ]
    then
        dns_setup_chain DROP

        local idx fwidx=1 dnslisten dnsif
        for idx in $(seq 1 $DNS_LISTEN_N)
        do
            eval dnslisten=\$DNS_LISTEN_${idx}
            case $dnslisten in
                IPV6_NET_*|*:*)
                    ;;
                *)
                    if translate_ipv4_to_dev $dnslisten dnsif
                    then
                        fw_insert_rule filter in-dns-udp $fwidx "if:$dnsif:any any $dnslisten ACCEPT" "accept UDP DNS requests on $dnslisten"
                        fw_insert_rule filter in-dns-tcp $fwidx "if:$dnsif:any any $dnslisten ACCEPT" "accept TCP DNS requests on $dnslisten"
                        fwidx=$((fwidx+1))
                    fi
                    ;;
            esac
        done
    fi
}

conf_firewall_dns6()
{
    if [ 0$DNS_LISTEN_N -ne 0 ]
    then
        dns_setup_chain6 DROP

        local idx fw6idx=1 dnslisten dnsif
        for idx in $(seq 1 $DNS_LISTEN_N)
        do
            eval dnslisten=\$DNS_LISTEN_${idx}
            case $dnslisten in
                IPV6_NET_*|*:*)
                    if translate_ipv6_to_dev $dnslisten dnsif
                    then
                        fw_insert_rule6 filter in-dns-udp $fw6idx "if:$dnsif:any any $dnslisten ACCEPT" "accept UDP DNS requests on $dnslisten"
                        fw_insert_rule6 filter in-dns-tcp $fw6idx "if:$dnsif:any any $dnslisten ACCEPT" "accept TCP DNS requests on $dnslisten"
                        fw6idx=$((fw6idx+1))
                    fi
                    ;;
            esac
        done
    fi
}

conf_firewall_dns ()
{
    [ "$PF_INPUT_ACCEPT_DEF" = "yes" ] && conf_firewall_dns4
    if [ "$OPT_IPV6" = yes ]
    then
        [ "$PF6_INPUT_ACCEPT_DEF" = "yes" ] && conf_firewall_dns6
    fi
}

########################################
# Functions dealing with subnet algebra
########################################

# Translates a hexadecimal digit to binary digits.
# Input:
#   $1 = hexadecimal digit (in lowercase)
# Output:
#   four binary digits representing passed hexadecimal digit
#
hex_to_bin()
{
    case $1 in
    0) echo 0000;;
    1) echo 0001;;
    2) echo 0010;;
    3) echo 0011;;
    4) echo 0100;;
    5) echo 0101;;
    6) echo 0110;;
    7) echo 0111;;
    8) echo 1000;;
    9) echo 1001;;
    a) echo 1010;;
    b) echo 1011;;
    c) echo 1100;;
    d) echo 1101;;
    e) echo 1110;;
    f) echo 1111;;
    esac
}

# Translates a dotted decimal IPv4 address to binary digits.
# Input:
#   $1 = dotted IPv4 address
# Output:
#   IPv4 address converted to binary digits (without separators).
#
ipv4_to_bin()
{
    local part result=
    for part in ${1//./ }
    do
        [ $part -lt 256 ] || return 1
        local hex=$(printf "%02x" $part)
        while [ -n "$hex" ]
        do
            result="${result}$(hex_to_bin "${hex:0:1}")"
            hex=${hex:1}
        done
    done
    echo "$result"
}

# Translates a binary IPv4 address to dotted decimal format.
# Input:
#   $1 = binary IPv4 address
# Output:
#   dotted decimal IPv4 address
#
bin_to_ipv4()
{
    local prefix=$1 groups=0 bits=0 value=0 digit result=
    # an IPv4 address contains four groups...
    while [ $groups -lt 4 ]
    do
        if [ -n "$prefix" ]
        then
            digit=${prefix:0:1}
            prefix=${prefix:1}
        else
            digit=0
        fi

        value=$((value*2+digit))
        bits=$((bits+1))
        # ...of values containing eight bits
        if [ $bits -eq 8 ]
        then
            result="${result}.${value}"
            bits=0
            value=0
            groups=$((groups+1))
        fi
    done
    echo "${result#.}"
}

# Removes a subnet from another net.
# Input:
#   $1 = subnet to remove
#   $2 = net from which to remove the subnet
# Output:
#   the remaining nets (if any)
#
remove_subnet_from_net()
{
    local subnet=$1 net=$2
    local subnet_sz=${subnet##*/} net_sz=${net##*/}

    # determine size of common prefix (if any)
    local prefix_sz
    if [ $subnet_sz -lt $net_sz ]
    then
        prefix_sz=$subnet_sz
    else
        prefix_sz=$net_sz
    fi

    # do nets have common prefix?
    local subnet_prefix=$(netcalc network ${subnet%/*}/${prefix_sz})
    local net_prefix=$(netcalc network ${net%/*}/${prefix_sz})
    if [ "$subnet_prefix" != "$net_prefix" ]
    then
        # no, subtraction is without effect
        echo "$net"
        return 0
    fi

    # is subnet really a subnet?
    if [ $subnet_sz -le $net_sz ]
    then
        # no, net is a subnet of net -- result is the empty list of nets
        return 0
    fi

    # transform (originally passed) subnet and net into binary format
    local subnet=$(ipv4_to_bin "$(netcalc network ${subnet})") \
        net=$(ipv4_to_bin "$(netcalc network ${net})")

    # cut off common prefix
    local prefix=${net:0:${net_sz}} prefix_len=${net_sz}
    net=${net:${net_sz}}
    subnet=${subnet:${net_sz}}
    subnet_sz=$((subnet_sz-net_sz))

    local digit result=
    while [ $subnet_sz -gt 0 ]
    do
        digit=${subnet:0:1}
        prefix_len=$((prefix_len+1))
        result="${result} $(bin_to_ipv4 "${prefix}$((1-digit))")/${prefix_len}"
        prefix="${prefix}${digit}"
        subnet=${subnet:1}
        subnet_sz=$((subnet_sz-1))
    done

    echo "$result"
}

# Removes a subnet from a list of nets.
# Input:
#   $1 = subnet to remove
#   $2... = nets from which to remove the subnet
# Output:
#   the remaining nets (if any)
#
remove_subnet_from_nets()
{
    local subnet=$1 net result=
    shift
    for net
    do
        result="$result $(remove_subnet_from_net $subnet $net)"
    done
    echo "$result"
}

####################################################
# Functions dealing with "bogus priv" functionality
####################################################

dns_private_subnets=/var/run/dns_private_subnets
dnsmasq_private_subnets=/etc/dnsmasq.d/dns_bogus_priv.conf

# Creates the "bogus priv" dnsmasq configuration file.
dns_create_private_subnets()
{
    if [ "$DNS_BOGUS_PRIV" = "yes" ]
    then
        local nets="10.0.0.0/8 192.168.0.0/16" i
        for i in $(seq 16 31)
        do
            nets="${nets} 172.${i}.0.0/16"
        done
        echo "$nets" > "$dns_private_subnets"

        for i in $(seq 1 ${DNS_FORWARD_PRIV_N:-0})
        do
            eval local net=\$DNS_FORWARD_PRIV_${i}
            dns_remove_private_subnet "$net"
        done
    fi
    dns_update_private_subnets
}

# Updates the "bogus priv" dnsmasq configuration file from the stored list of
# private subnets excluded from being forwarded when doing reverse DNS lookups.
dns_update_private_subnets()
{
    local net rev
    for net in $(cat "$dns_private_subnets")
    do
        for rev in $(netcalc dnsrev "$net")
        do
            echo "local=/$rev/"
        done
    done > "$dnsmasq_private_subnets"
}

# Allows DNS PTR lookups of hosts in passed private subnet.
# Input:
#   $1 = network (with CIDR mask) where reverse DNS lookups should be handled
#        by forwarding
dns_remove_private_subnet()
{
    local nets=$(cat "$dns_private_subnets")
    nets=$(remove_subnet_from_nets "$1" $nets)
    echo "$nets" > $dns_private_subnets
}
