#!/bin/sh
#----------------------------------------------------------------------------
# ipv6-interface-setup.sh                                        __FLI4LVER__
#
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

# $1 = file containing IPv6 network data
# $2 = prefix to delete during setup (may be empty)

. /etc/boot.d/logging.inc
. /etc/boot.d/networking.inc

netfile="$1"
prefixdel="$2"

if [ ! -f "$netfile" ]
then
    log_error "$(basename $0): IPv6 network information file '$netfile' does not exist"
    exit 1
fi

. "$netfile"

netindex=$(basename "$netfile" .conf)
netname="IPV6_NET_${netindex}"
eval $(grep "^$netname=" /etc/rc.cfg)
eval net=\$$netname
nets=$(circuit_resolve_address "$net" ipv6)

if [ -n "$prefixdel" ]
then
    prefixdellen=${prefixdel##*/}
    addrdel=$(netcalc combine $prefixdel $netaddr)
    nets="$nets ${addrdel%/*}/$netlen"
fi

if  [ -z "$nets" ]
then
    log_error "$(basename $0): Translation of $netname failed"
    exit 2
fi

# assign defaults
advvalidlft_global=$advvalidlft
advpreflft_global=$advpreflft
: ${advvalidlft_global:='86400'}
: ${advpreflft_global:='14400'}

dnsdev=$(echo -n $dev | sed 's/\./-vlan-/;s/:/-alias-/')
eval $(grep "^\(HOSTNAME\|DOMAIN_NAME\|OPT_DNS\|DNS_SUPPORT_IPV6\|DNS_LISTEN_N\)=" /etc/rc.cfg)
if [ "$OPT_DNS" = "yes" -a "$DNS_SUPPORT_IPV6" = "yes" ]
then
    eval $(grep "^HOST_\(N\|[0-9]\+_\(IP[46]\|IP6_NET\|MAC\)\)=" /etc/rc.cfg)
fi

if [ "$adv" = "yes" ]
then
    mkdir -p /var/run/radvd/legacy
    > /var/run/radvd/legacy/$netindex.conf
fi

for net in $nets
do
    netaddr=${net%/*}
    netlen=${net##*/}
    netprefix=$(netcalc network $net)/$netlen

    op=up
    if [ -n "$prefixdellen" ]
    then
        prefixdelfromaddr=$(netcalc network $netaddr/$prefixdellen)/$prefixdellen
        [ "$prefixdelfromaddr" = "$prefixdel" ] && op=down
    fi

    if [ "$op" = up ]
    then
        set -- $(circuit_get_prefix_by_address $netaddr ipv6)
        circ_id=$1
        orig_prefix=$2
        start=$3
        [ -n "$4" ] && prefixvalidlft=$4
        [ -n "$5" ] && prefixpreflft=$5

        if [ -n "$circ_id" ]
        then
            now=$(date +%s)
            if [ $prefixvalidlft != "forever" ]
            then
                if [ $((start+prefixvalidlft)) -lt $now ]
                then
                    prefixvalidlft=0
                else
                    prefixvalidlft=$((prefixvalidlft+start-now))
                fi
            fi
            if [ $prefixpreflft != "forever" ]
            then
                if [ $((start+prefixpreflft)) -lt $now ]
                then
                    prefixpreflft=0
                else
                    prefixpreflft=$((prefixpreflft+start-now))
                fi
            fi
        fi
    else
        circ_id=
        prefixvalidlft=0
        prefixpreflft=0
    fi

    # lifetime handling: if a prefix lifetime exists, it overrides the
    # configuration found in IPV6_NET_x_ADVERTISE_(VALID|PREF)_LIFETIME
    advvalidlft=$advvalidlft_global
    advpreflft=$advpreflft_global
    [ -n "$prefixvalidlft" ] && advvalidlft=$prefixvalidlft
    [ -n "$prefixpreflft" ]  && advpreflft=$prefixpreflft
    : ${prefixvalidlft:=forever}
    : ${prefixpreflft:=forever}

    if [ "$adv" = "yes" ]
    then
        {
            if [ $advvalidlft = "forever" ]
            then
                validlifetime=infinity
            else
                validlifetime=$advvalidlft
            fi
            if [ $advpreflft = "forever" ]
            then
                preferredlifetime=infinity
            else
                preferredlifetime=$advpreflft
            fi

            echo "    prefix $netprefix {"
            echo "      AdvValidLifetime $validlifetime;"
            echo "      AdvPreferredLifetime $preferredlifetime;"

            if [ -n "$circ_id" -a $advvalidlft != "forever" \
                -a $advpreflft != "forever" ]
            then
                echo "      DecrementLifetimes on;"
            fi

            echo "    };"

            if [ "$adv_dns" = "yes" -a "$op" = up ]
            then
                echo "    RDNSS $netaddr {"
                echo "    };"
            fi

            if [ "$op" = up ]
            then
                log_info "$(basename $0): $netname: advertising prefix $netprefix on $dev for $advvalidlft/$advpreflft seconds"
            else
                log_info "$(basename $0): $netname: stopped advertising prefix $netprefix on $dev"
            fi
        } >> /var/run/radvd/legacy/$netindex.conf
    fi

    if [ "$OPT_DNS" = "yes" -a "$DNS_SUPPORT_IPV6" = "yes" -a "$DNS_LISTEN_N" != "0" ]
    then
        if [ "$adv_dns" = "yes" ]
        then
            if [ "$op" = up ]
            then
                if [ ! -f /etc/dnsmasq.d/dns_ipv6.conf ] ||
                    ! grep -q "^listen-address=$netaddr$" /etc/dnsmasq.d/dns_ipv6.conf
                then
                    mkdir -p /etc/dnsmasq.d
                    echo "listen-address=$netaddr" >> /etc/dnsmasq.d/dns_ipv6.conf
                    > /var/run/reload.dnsmasq
                fi
            elif [ -f /etc/dnsmasq.d/dns_ipv6.conf ]
            then
                sed -i "/^listen-address=$netaddr$/d" /etc/dnsmasq.d/dns_ipv6.conf
                > /var/run/reload.dnsmasq
            fi
        fi
    fi

    if [ "$op" = up ]
    then
        ip -6 addr replace $net dev $dev valid_lft $prefixvalidlft preferred_lft $prefixpreflft
        log_info "$(basename $0): $netname: address $net added to $dev for $prefixvalidlft/$prefixpreflft seconds"
    else
        ip -6 addr del $net dev $dev
        log_info "$(basename $0): $netname: address $net removed from $dev"
    fi

    if [ -f /var/run/ipv6.hostname ]
    then
        . /var/run/ipv6.hostname
    else
        host_net=
    fi

    if [ "$op" = up ] && ! grep -q "^$netaddr " /etc/hosts
    then
        if [ "$netname" = "$host_net" ]
        then
            echo "$netaddr $HOSTNAME.$DOMAIN_NAME $HOSTNAME $host_aliases" >> /etc/hosts
        fi
        echo "$netaddr $canonical $HOSTNAME-$dnsdev.$DOMAIN_NAME $HOSTNAME-$dnsdev" >> /etc/hosts
    elif [ "$op" != up ]
    then
        if [ "$netname" = "$host_net" ]
        then
            sed -i "/^$netaddr $HOSTNAME.$DOMAIN_NAME $HOSTNAME $host_aliases$/d" /etc/hosts
        fi
        sed -i "/^$netaddr $canonical $HOSTNAME-$dnsdev.$DOMAIN_NAME $HOSTNAME-$dnsdev$/d" /etc/hosts
    fi

    if [ "$OPT_DNS" = "yes" -a "$DNS_SUPPORT_IPV6" = "yes" -a $netlen -eq 64 ]
    then
        for idx in $(seq 1 $HOST_N)
        do
            eval ip6='$HOST_'$idx'_IP6'
            eval ip6net='$HOST_'$idx'_IP6_NET'
            eval mac='$HOST_'$idx'_MAC'
            : ${ip6net:="IPV6_NET_1"}

            if [ "$ip6" = "auto" -a "$ip6net" = "$netname" -a -n "$mac" ]
            then
                eval ip4='$HOST_'$idx'_IP4'
                if translate_ip_net $ip4 HOST_${idx}_IP4
                then
                    ip4=$res
                fi
                ip4=$(echo "$ip4" | sed 's,\.,\\.,g')

                # compute EUI-64 from MAC
                set -- $(echo -n $mac | cut -d ':' -f 1 | sed 's/^\(.\)\(.\).*$/\1 \2/')
                first=$1
                fliplocalbit $2

                eui="$first$result$(echo -n $mac | cut -d ':' -f 2,3)ff:fe$(echo -n $mac | cut -d ':' -f 4,5)$(echo -n $mac | cut -d ':' -f 6)"
                addr=$(netcalc combine $netprefix ::$eui)
                addr=${addr%/*}
                for f in /etc/hosts /etc/hosts.d/hosts.*
                do
                    [ -f "$f" ] || continue
                    if [ "$op" = up ]
                    then
                        if ! grep -q "^$addr " $f
                        then
                            grep "^$ip4 " $f | sed "s/^$ip4 /$addr /" >> $f
                        fi
                    else
                        sed -i "/^$addr /d" $f
                    fi
                done

                > /var/run/reload.dnsmasq
            fi
        done
    fi
done

if [ "$adv" = "yes" ]
then
    > /var/run/reload-legacy.radvd
fi

exit 0
