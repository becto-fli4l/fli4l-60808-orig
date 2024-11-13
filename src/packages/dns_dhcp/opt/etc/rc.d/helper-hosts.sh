#!/bin/sh
#------------------------------------------------------------------------------
# /etc/rc.d/helper-hosts.sh - configure host-names
#
# Creation:     2007-02-03  LanSpezi
# Last Update:  $Id$
#------------------------------------------------------------------------------

# Generates lines for inclusion into a hosts file (e.g. /etc/hosts) given a
# variable prefix pointing to a host. This function assumes that the variables
# <host-var-prefix>_IP4 and/or <host-var-prefix>_IP6 are set to the IPv4
# and/or IPv6 address of the host, respectively.
#
# Input:
#   $1    = host variable prefix (e.g. HOST_1)
#   $2... = DNS name(s)
# Output:
#   Zero to many lines conforming to the format of /etc/hosts.
create_dns_entry()
{
    local hostvarprefix=$1 res
    shift

    eval local ipv4=\$${hostvarprefix}_IP4
    if translate_ip_net $ipv4 ${hostvarprefix}_IP4 "" "" addr
    then
        echo "$res $*"
        eval ${hostvarprefix}_IP4="$res"
    fi
    if [ "$OPT_DNS" = "yes" -a "$DNS_SUPPORT_IPV6" = "yes" ]
    then
        eval local ipv6=\$${hostvarprefix}_IP6
        if [ -n "$ipv6" -a "$ipv6" != "auto" ]
        then
            if translate_ip6_net $ipv6 ${hostvarprefix}_IP6 "" "" addr
            then
                echo "$res $*"
                eval ${hostvarprefix}_IP6="$res"
            fi
        fi
    fi
}

create_dns_hosts ()
{
    if [ "$OPT_DNS" = "yes" ]
    then
        hostfile="/etc/hosts.d/hosts.dns"
    else
        hostfile="/etc/hosts"
    fi

    if [ 0$HOST_N -ne 0 ]
    then

        log_info "writing HOSTS to $hostfile ..."

        mkdir -p /etc/dnsmasq.d

        {
            cat <<EOF
#----------------------------------------------------------------------------
# $hostfile
# automatically created by rc001.hosts
#----------------------------------------------------------------------------
EOF
            [ 0$HOST_N -eq 0 ] || for i in `seq 1 $HOST_N`
            do
                eval hostname='$HOST_'$i'_NAME'

                eval host_domain='$HOST_'$i'_DOMAIN'
                if [ "x$host_domain" != x ]
                then
                    domain="$host_domain"
                else
                    domain="$DOMAIN_NAME"
                fi

                eval alias_n='$HOST_'$i'_ALIAS_N'
                [ 0"$alias_n" -eq 0 ] || for j in `seq 1 $alias_n`
                do
                    eval alias='$HOST_'$i'_ALIAS_'$j
                    case $alias
                    in
                        *.*) ;;
                        *)   alias="$alias.$domain" ;;
                    esac

                    # check whether alias is equal to any known host name
                    alias_lowercase=$(echo "$alias" | tr 'A-Z' 'a-z')
                    found=
                    for k in `seq 1 $HOST_N`
                    do
                        eval hostname2='$HOST_'$k'_NAME'
                        eval host_domain2='$HOST_'$k'_DOMAIN'
                        if [ -n "$host_domain2" ]
                        then
                            hostname2="$hostname2.$host_domain2"
                        else
                            hostname2="$hostname2.$DOMAIN_NAME"
                        fi

                        if [ "$alias_lowercase" = $(echo "$hostname2" | tr 'A-Z' 'a-z') ]
                        then
                            log_warn "Ignoring CNAME $alias (defined by HOST_${i}_ALIAS_${j}) because it is the same as the FQDN $hostname2 (defined by HOST_${k}_NAME)"
                            found=1
                            break
                        fi
                    done
                    
                    if grep -iq "=$alias," /etc/dnsmasq.d/cname.conf 2>/dev/null
                    then
                        log_warn "Ignoring CNAME $alias (defined by HOST_${i}_ALIAS_${j}) because it has been already defined by some other host alias"
                        found=1
                    fi

                    if [ -z "$found" ]
                    then
                        echo "cname=$alias,$hostname.$domain" >> /etc/dnsmasq.d/cname.conf
                    fi
                done

                create_dns_entry HOST_${i} "$hostname.$domain" "$hostname"
            done
        } >> $hostfile

        if [ -f /etc/dnsmasq.d/cname.conf ]
        then
            chown dns:dns /etc/dnsmasq.d/cname.conf
            chmod 600 /etc/dnsmasq.d/cname.conf
        fi

    fi
}

create_dns_hosts_extra ()
{
    if [ "$OPT_DNS" = "yes" ]
    then
        hostfile="/etc/hosts.d/hosts.extra"
    else
        hostfile="/etc/hosts"
    fi

    if [ 0$HOST_EXTRA_N -ne 0 ]
    then
        log_info "writing EXTRA_HOSTS to $hostfile ..."

        {
            cat <<EOF
#----------------------------------------------------------------------------
# $hostfile
# automatically created by rc001.hosts
#----------------------------------------------------------------------------
EOF
            [ 0$HOST_EXTRA_N -eq 0 ] || for i in `seq 1 $HOST_EXTRA_N`
            do
                eval hostname='$HOST_EXTRA_'$i'_NAME'
                create_dns_entry HOST_EXTRA_${i} "$hostname"
            done
        } >> $hostfile
    fi
}
