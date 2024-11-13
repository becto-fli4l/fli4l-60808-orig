#!/bin/sh

#----------------------------------------------------------------------------
# /srv/www/admin/status_dhcp-dnsmasq.cgi
#
# Creation:     LanSpezi 
# Last Update:  $Id$
#----------------------------------------------------------------------------

## functions
split_dhcp_host_data ()
{
    eval mac=`echo $line | cut -d "=" -f2 | cut -d "," -f1`
    if [ `echo $mac | cut -c 3` != ":" ]
    then
        mac="<hostname>"
    fi
    eval ipaddr=`echo $line | cut -f2 -d ","`
    if [ `echo $ipaddr | cut -c 3` = ":" ]
    then
        eval mac2=`echo $line | cut -f2 -d ","`
        eval ipaddr=`echo $line | cut -f3 -d ","`
        eval hostname=`echo $line | cut -f4 -d ","`
        eval lease_time=`echo $line | cut -d "," -f5`
    else
        mac2="-"
        eval hostname=`echo $line | cut -f3 -d ","`
        eval lease_time=`echo $line | cut -d "," -f4`
    fi
}

split_dhcp_range_data ()
{
    id_net=$1
    unset range_start
    unset range_end
    unset lease_time
    unset subnet_mask
    unset def_router
    unset broadcast
    unset dns_server
    unset dns_domain
    unset ntp_server
    
    while read line
    do
        case $line in
            dhcp-range*static)
                range_type="static"
                range_start="-"
                range_end="-"
                lease_time="-"
                ;;
            dhcp-range*)
                range_type="dynamic"
                eval range_start=`echo $line | cut -d "," -f2`
                eval range_end=`echo $line | cut -d "," -f3`
                eval lease_time=`echo $line | cut -d "," -f4`
                ;;
            dhcp-option*,1,*)
                eval subnet_mask=`echo $line | cut -d "," -f3`
                ;;
            dhcp-option*,3,*)
                eval def_router=`echo $line | cut -d "," -f3`
                ;;
            dhcp-option*,28,*)
                eval broadcast=`echo $line | cut -d "," -f3`
                ;;
            dhcp-option*,6,*)
                eval dns_server=`echo $line | cut -d "," -f3`
                ;;
            dhcp-option*,15,*)
                eval dns_domain=`echo $line | cut -d "," -f3`
                ;;
            dhcp-option*,42,*)
                eval ntp_server=`echo $line | cut -d "," -f3`
                ;;
            esac               
    done < /etc/dnsmasq.d/dhcp_range_NET${id_net}.conf
}

#main

. /var/run/dhcp_leases_dir
    
case $DHCP_LEASES_DIR in
    */)
        LEASES_FILE="${DHCP_LEASES_DIR}dnsmasq.leases"
        ;;
    *)
        LEASES_FILE="$DHCP_LEASES_DIR/dnsmasq.leases"
        ;;
esac

. /srv/www/include/cgi-helper
. /srv/www/include/cgi-helper-ip4

if [ -f /tmp/template_dhcp.txt ]
then
    rm /tmp/template_dhcp.txt
fi

check_rights "status" "change"

case $FORM_action in
    Download)
        check_rights "status" "change"
        if [ -f /etc/dnsmasq.d/dhcp_hosts_NET*.wwwconf ]
        then
            http_header download "ctype=text/plain;filename=dhcp_hosts.txt"
            {
            echo "#"
            echo "# please copy needed host-data to your config/dns_dhcp.txt and replace x by correct numbers"
            echo "#"
            echo "# do not forget to correct HOST_N"
            echo "#"
            } >> /tmp/template_dhcp.txt
            while read netid
            do
                if [ -f /etc/dnsmasq.d/dhcp_hosts_NET${netid}.wwwconf ]
                then
                    while read line
                    do
                        {
                        split_dhcp_host_data
                        echo "HOST_x_NAME='$hostname'"
                        echo "HOST_x_IP4='$ipaddr'"
                        echo "HOST_x_MAC='$mac'"
                        echo "HOST_x_DHCPTYP='mac'"
                        echo ""
                        } >> /tmp/template_dhcp.txt
                    done < /etc/dnsmasq.d/dhcp_hosts_NET${netid}.wwwconf
                fi
            done < /var/run/dhcp_range_net.list
            cat /tmp/template_dhcp.txt
        else
            ref="refresh=5;url=$myname?action=view"
            show_html_header "$_STAT_DHCP_MAINTITLE ($_STAT_DHCP_DOWNLOAD_HEAD)" "${ref}"
            show_info "$_STAT_DHCP_DOWNLOAD_HEAD" "$_STAT_DHCP_DOWNLOAD_INFO"
            show_html_footer
        fi
        ;;
        
    add)
        check_rights "status" "change"
        ref="refresh=15;url=$myname?action=view"
        show_html_header "$_STAT_DHCP_MAINTITLE ($_STAT_DHCP_ADD_HEAD)" "${ref}"
        #todo:
        # - check data and add to /etc/dnsmasq.d/dhcp_hosts${netid}.wwwconf
        adderror=0

        if [ "x$FORM_ip1" = "x" -o "x$FORM_ip2" = "x" -o "x$FORM_ip3" = "x" -o "x$FORM_ip4" = "x" ]
        then
            show_error "$_STAT_DHCP_ERROR $_STAT_DHCP_IPADDR" "$_STAT_DHCP_IP_EMPTYFIELD"
            echo "<br />"
            adderror=1
        else
            #first check for same IP
            tmp_ip=`echo "${FORM_ip1}.${FORM_ip2}.${FORM_ip3}.${FORM_ip4}"`
            if grep -q ,$tmp_ip, /etc/dnsmasq.d/dhcp_hosts_NET*
            then
                show_error "$_STAT_DHCP_ERROR $_STAT_DHCP_IPADDR" "$_STAT_DHCP_ERR_IP_DOUBLE"
                echo "<br />"
                adderror=1
            else
                #check for valid ip-adress - only valid ip-adresses are normalized
                ip4_normalize ${tmp_ip}
                FORM_ip=$res
                                                                
                if [ "x$FORM_ip" = "x" ]
                then
                    show_error "$_STAT_DHCP_ERROR $_STAT_DHCP_IPADDR" "$_STAT_DHCP_ERR_IP_FORMAT"
                    echo "<br />"
                    adderror=1
                else
                    split_dhcp_range_data $FORM_netid
                    eval ip_bcast=`netcalc broadcast ${FORM_ip} ${subnet_mask}`
                    eval net_network=`netcalc network ${broadcast} ${subnet_mask}`
                    if [ "$ip_bcast" != "$broadcast" ]
                    then
                        show_error "$_STAT_DHCP_ERROR $_STAT_DHCP_IPADDR" "$_STAT_DHCP_ERR_IP_NOTNET"
                        echo "<br />"
                        adderror=1
                    fi
                    # check if ip is bcast or network-adress
                    if [ "${FORM_ip}" = "$broadcast" ]
                    then
                        show_error "$_STAT_DHCP_ERROR $_STAT_DHCP_IPADDR" "$_STAT_DHCP_ERR_IP_BCAST"
                        echo "<br />"
                        adderror=1
                    fi
                    if [ "${FORM_ip}" = "$net_network" ]
                    then
                        show_error "$_STAT_DHCP_ERROR $_STAT_DHCP_IPADDR" "$_STAT_DHCP_ERR_IP_NET"
                        echo "<br />"
                        adderror=1
                    fi
                    # check for same IP after normalize of IP
                    if grep -q ,$FORM_ip, /etc/dnsmasq.d/dhcp_hosts_NET*
                    then
                        show_error "$_STAT_DHCP_ERROR $_STAT_DHCP_IPADDR" "$_STAT_DHCP_ERR_IP_DOUBLE"
                        echo "<br />"
                        adderror=1
                    fi
                    #check if IP is in DHCP-Range
                    if [ "$range_type" = "dynamic" ]
                    then
                        ip4_isindhcprange $FORM_ip $range_start $range_end
                        if [ $? = 0 ]
                        then
                            show_error "$_STAT_DHCP_ERROR $_STAT_DHCP_IPADDR" "$_STAT_DHCP_ERR_IP_RANGE"
                            echo "<br />"
                            adderror=1
                        fi
                        
                    fi
                fi
            fi
        fi

        if [ "x`echo $FORM_host | sed 's/[a-zA-Z0-9\-]//g'`" != "x" ]
        then
            show_error "$_STAT_DHCP_ERROR $_STAT_DHCP_HOSTNAME" "$_STAT_DHCP_ERR_HOST_VALIDATE"
            echo "<br />"
            adderror=1
        fi

        if [ "x$FORM_host" = "x" ]
        then
            show_error "$_STAT_DHCP_ERROR $_STAT_DHCP_HOSTNAME" "$_STAT_DHCP_HOST_EMPTY"
            echo "<br />"
            adderror=1
        else
            #check for same hostname in any config-file
            if grep -i -q ,$FORM_host, /etc/dnsmasq.d/dhcp_hosts_NET*
            then
                show_error "$_STAT_DHCP_ERROR $_STAT_DHCP_HOSTNAME" "$_STAT_DHCP_ERR_HOST_DOUBLE"
                echo "<br />"
                adderror=1
            fi
        fi

        if [ "x$FORM_mac" = "x" ]
        then
            show_error "$_STAT_DHCP_ERROR $_STAT_DHCP_MACADDR" "$_STAT_DHCP_MAC_EMPTY"
            echo "<br />"
            adderror=1
        else
            #check for same mac-addr in config-file of the net
            if grep -i -q $FORM_mac, /etc/dnsmasq.d/dhcp_hosts_NET${FORM_netid}*
            then
                show_error "$_STAT_DHCP_ERROR $_STAT_DHCP_MACADDR" "$_STAT_DHCP_ERR_MAC_DOUBLE"
                echo "<br />"
                adderror=1
            else
                #                             00 00 00 11 11 11
                #check mac for correct format 12:45:78:01:34:67
                macok=1
                if ! echo $FORM_mac | grep -q -i '[0-9A-F]\{2\}\(:[0-9A-F]\{2\}\)\{5\}'
                then
                    macok=0
                fi
                if [ $macok = 0 ]
                then
                    show_error "$_STAT_DHCP_ERROR $_STAT_DHCP_MACADDR" "$_STAT_DHCP_ERR_MAC_FORMAT"
                    echo "<br />"
                    adderror=1
                fi
            fi
        fi
        if [ "x$FORM_leasetime" = "x" ]
        then
            show_error "$_STAT_DHCP_ERROR $_STAT_DHCP_LEASETIME" "$_STAT_DHCP_LEASETIME_EMPTY"
            echo "<br />"
            adderror=1
        else
            if [ ! 0$FORM_leasetime -gt 120 ]
            then
                show_error "$_STAT_DHCP_ERROR $_STAT_DHCP_LEASETIME" "$_STAT_DHCP_ERR_LEASETIME"
                echo "<br />"
                adderror=1
            fi
        fi
        
        ##check if ip or hostname has an entry in /etc/hosts.d/hosts.*
        if [ "x$FORM_ip" != "x" ]
        then
            if grep -q -i "$FORM_ip " /etc/hosts.d/hosts.*
            then
                eval hostsentry=`grep -i "$FORM_ip " /etc/hosts.d/hosts.*`
                if ! echo $hostsentry | grep -q -i $FORM_host
                then
                    show_error "$_STAT_DHCP_ERROR $_STAT_DHCP_HOSTSFILE" "$_STAT_DHCP_ERR_NEWIPHASDNSENTRY"
                    echo "<br />"
                    adderror=1
                fi
            fi
        fi
        
        if [ "x$FORM_host" != "x" ]
        then
            if grep -q -i " $FORM_host" /etc/hosts.d/hosts.*
            then
                eval hostsentry=`grep -i " $FORM_host" /etc/hosts.d/hosts.*`
                if ! echo $hostsentry | grep -q -i $FORM_ip
                then
                    show_error "$_STAT_DHCP_ERROR $_STAT_DHCP_HOSTSFILE" "$_STAT_DHCP_ERR_NEWHOSTHASDNSENTRY"
                    echo "<br />"
                    adderror=1
                fi
            fi
        fi

        # all checks done
        if [ $adderror = 1 ]
        then
            show_error "$_STAT_DHCP_ERROR $_STAT_DHCP_ERR_DATA" "$_STAT_DHCP_ERR_NOHOSTADDED"
            echo "<br />"
        else
            dyn_dhcp_host=0
            # check if hostname exist in lease-file 
            if grep -q -i " $FORM_host " $LEASES_FILE
            then
                dyn_dhcp_host=1
                grep -v -i " $FORM_host " $LEASES_FILE > /tmp/new_lease.dhcp
            fi
            
            # check if mac exist in lease-file             
            if grep -q -i " $FORM_mac " $LEASES_FILE
            then
                dyn_dhcp_host=1
                grep -v -i " $FORM_mac " $LEASES_FILE > /tmp/new_lease.dhcp
            fi
    
            if [ $dyn_dhcp_host = 1 ]
            then
                show_info "$_STAT_DHCP_INFO $_STAT_DHCP_INFO_DHCPHOST" "$_STAT_DHCP_INFO_RENEWNEEDED"
                cat /tmp/new_lease.dhcp > $LEASES_FILE
                rm /tmp/new_lease.dhcp
            fi
            
            # format of line to append of config-file
            # dhcp-host=00:01:01:01:01:01,192.168.6.1,testhost,86400,net:NET1
            echo "dhcp-host=$FORM_mac,$FORM_ip,$FORM_host,$FORM_leasetime,net:NET$FORM_netid" >> /etc/dnsmasq.d/dhcp_hosts_NET$FORM_netid.wwwconf
            chown dns:dns /etc/dnsmasq.d/dhcp_hosts_NET${FORM_netid}.wwwconf
            chmod 600 /etc/dnsmasq.d/dhcp_hosts_NET${FORM_netid}.wwwconf
            
            #add entry to /etc/hosts.d/hosts.dhcp if needed
            if ! grep -q -i "$FORM_ip " /etc/hosts.d/hosts.*
            then
                echo "$FORM_ip $FORM_host" >> /etc/hosts.d/hosts.dhcp
            fi
            
            show_info "$_STAT_DHCP_ADD_HEAD" "$_STAT_DHCP_ADD_HOSTADDED"
            
            echo "1" > /tmp/reload_dnsmasq.wwwflag
        fi
        show_html_footer
        ;;

    delete)
        check_rights "status" "change"
        ref="refresh=5;url=$myname?action=view"
        show_html_header "$_STAT_DHCP_MAINTITLE ($_STAT_DHCP_DEL_HEAD)" "${ref}"
        show_info "$_STAT_DHCP_DEL_HEAD" "$_STAT_DHCP_DEL_HOSTREMOVED"
        echo ""
        
        if grep -q -i " $FORM_hostname " $LEASES_FILE
        then
            grep -v -i " $FORM_hostname " $LEASES_FILE > /tmp/new_lease.dhcp
            cat /tmp/new_lease.dhcp > $LEASES_FILE
            rm /tmp/new_lease.dhcp
        fi
        
        grep -v ",$FORM_hostname," /etc/dnsmasq.d/dhcp_hosts_NET${FORM_net}.wwwconf > /tmp/wwwdhcp.$$
        set -- `ls -l /tmp/wwwdhcp.$$`
        if [ "$5" = "0" ]
        then
            rm /etc/dnsmasq.d/dhcp_hosts_NET${FORM_net}.wwwconf
        else
            mv /tmp/wwwdhcp.$$ /etc/dnsmasq.d/dhcp_hosts_NET${FORM_net}.wwwconf
            chown dns:dns /etc/dnsmasq.d/dhcp_hosts_NET${FORM_net}.wwwconf
            chmod 600 /etc/dnsmasq.d/dhcp_hosts_NET${FORM_net}.wwwconf
        fi
        rm /tmp/wwwdhcp.$$
        
        #remove entry from /etc/hosts.d/hosts.dhcp
        if grep -q -i " $FORM_hostname" /etc/hosts.d/hosts.dhcp
        then
            grep -v -i " $FORM_hostname" /etc/hosts.d/hosts.dhcp > /tmp/wwwdhcphosts.$$
            cat /tmp/wwwdhcphost.$$ > /etc/hosts.d/hosts.dhcp
            rm /tmp/wwwdhcphosts.$$
        fi
        
        echo "1" > /tmp/reload_dnsmasq.wwwflag
        show_html_footer
        ;;

    *|view)
        if [ -f /tmp/reload_dnsmasq.wwwflag ]
        then
            ref="refresh=5;url=$myname?action=view"
            show_html_header "$_STAT_DHCP_MAINTITLE ($_STAT_DHCP_RELOAD_HEAD)" "${ref}"
            rm /tmp/reload_dnsmasq.wwwflag
            killall dnsmasq
            show_info "$_STAT_DHCP_RELOAD_HEAD" "$_STAT_DHCP_MSG_RELOAD"
            echo "<br />"
            sleep 2
            dnsmasq
        else
            show_html_header "$_STAT_DHCP_MAINTITLE " "${ref}"
        fi

        # check for exist of lease file
        if [ ! -f $LEASES_FILE ]
        then
            show_error "$_STAT_DHCP_ERROR" "$_STAT_DHCP_LEASE_NOLEASEFILE"
        else
            if [ -s $LEASES_FILE ]
            then
                sort -n $LEASES_FILE > /tmp/leases.sort.$$
                show_tab_header "$_STAT_DHCP_LEASE_TABHEAD" "no"
                echo "<table class=\"normtable\">"
                echo "<tr>"
                echo "<th><center>$_STAT_DHCP_HOSTNAME</center></th>"
                echo "<th><center>$_STAT_DHCP_IPADDR</center></th>"
                echo "<th><center>$_STAT_DHCP_MACADDR</center></th>"
                echo "<th><center>$_STAT_DHCP_LEASE_END</center></th>"
                echo "<th><center>$_STAT_DHCP_NETWORK</center></th>"
                echo "</tr>"

                datediff=`date +%z`
                datediff2=`expr $datediff "*" 36`
            
                for fa in /tmp/dhcp_hosts_NET*.add
                do
                    rm ${fa}
                done
            
                while read line
                do
                    set -f
                    set -- `echo $line`
                    time=$1
                    mac=$2
                    ip=$3
                    hostname=$4
                    set +f
            
                    if [ $time -eq 0 ]
                    then
                        time2="never"
                    else
                        time=`expr $time "+" $datediff2`
                        time2=`date -d "1970.1.1-0:0:$time" +%d.%m.%Y'&nbsp;'%H:%M:%S`
                    fi
                    while read netid
                    do
                        split_dhcp_range_data $netid
                        ip_bcast=`netcalc broadcast $ip $subnet_mask`
                        if [ "$broadcast" = "$ip_bcast" ]
                        then
                            net="IP_NET_${netid}"
                            if ! grep -q -i ",$ip," /etc/dnsmasq.d/dhcp_hosts_NET${netid}.*conf
                            then
                                if ! grep -q ",${hostname}," /tmp/dhcp_hosts_NET${netid}.add
                                then
                                    echo "add-posible="$mac",xxx.xxx.xxx.xxx,$hostname,3600,NET${netid}" >> /tmp/dhcp_hosts_NET${netid}.add
                                fi
                            fi
                            break
                        fi
                    done < /var/run/dhcp_range_net.list 
                    echo "<tr>"
                    echo "<td><center>$hostname</center></td>"
                    echo "<td><center>$ip</center></td>"
                    echo "<td><center>$mac</center></td>"
                    echo "<td><center>$time2</center></td>"
                    echo "<td><center>&nbsp;&nbsp;&nbsp;$net&nbsp;&nbsp;&nbsp;</center></td>"
                    echo "</tr>"
                done < /tmp/leases.sort.$$
                rm /tmp/leases.sort.$$
                echo "</table>"
                show_tab_footer
            else
                show_info "$_STAT_DHCP_INFO" "$_STAT_DHCP_LEASE_NOENTRY"
            fi
        fi

        if [ -z "`ls /etc/dnsmasq.d/dhcp_range_NET*.conf 2>/dev/null`" ]
        then
            show_info "$_STAT_DHCP_INFO=" "$_STAT_DHCP_ERR_NORANGE"
        else
            echo "<br />"
            show_tab_header "$_STAT_DHCP_RANGE_TABHEAD" "no"
            echo "<table class=\"normtable\">"
            echo "<tr>"
            echo "<th><center>$_STAT_DHCP_NETWORK</center></th>"
            echo "<th><center>$_STAT_DHCP_RANGE_TYPE</center></th>"
            echo "<th><center>$_STAT_DHCP_RANGE_START</center></th>"
            echo "<th><center>$_STAT_DHCP_RANGE_END</center></th>"
            echo "<th><center>$_STAT_DHCP_LEASETIME<br />[sec]</center></th>"
            echo "<th><center>$_STAT_DHCP_RANGE_MASK</center></th>"
            echo "<th><center>$_STAT_DHCP_RANGE_BCAST</center></th>"
            echo "<th><center>$_STAT_DHCP_RANGE_DEFROUTER</center></th>"
            echo "<th><center>$_STAT_DHCP_RANGE_DNSDOMAIN</center></th>"
            echo "<th><center>$_STAT_DHCP_RANGE_DNSSRV</center></th>"
            echo "<th><center>$_STAT_DHCP_RANGE_NTPSRV</center></th>"
            echo "</tr>"
    
            while read netid
            do
                if [ -f /etc/dnsmasq.d/dhcp_range_NET${netid}.conf ]
                then
                    split_dhcp_range_data $netid
    
                    echo "<tr>"
                    echo "<td><center>&nbsp;&nbsp;&nbsp;IP_NET_$netid&nbsp;&nbsp;&nbsp;</center></td>"
                    echo "<td><center>$range_type</center></td>"
                    echo "<td><center>$range_start</center></td>"
                    echo "<td><center>$range_end</center></td>"
                    echo "<td><center>$lease_time</center></td>"
                    echo "<td><center>$subnet_mask</center></td>"
                    echo "<td><center>$broadcast</center></td>"
                    echo "<td><center>$def_router</center></td>"
                    echo "<td><center>$dns_domain</center></td>"
                    echo "<td><center>$dns_server</center></td>"
                    echo "<td><center>$ntp_server</center></td>"
                    echo "</tr>"
                else
                    echo "<tr>"
                    echo "<td><center>&nbsp;&nbsp;&nbsp;IP_NET_$netid&nbsp;&nbsp;&nbsp;</center></td>"
                    echo "<td><center>No DHCP-Range</center></td>"
                    echo "<td><center>-</center></td>"
                    echo "<td><center>-</center></td>"
                    echo "<td><center>-</center></td>"
                    echo "<td><center>-</center></td>"
                    echo "<td><center>-</center></td>"
                    echo "<td><center>-</center></td>"
                    echo "<td><center>-</center></td>"
                    echo "<td><center>-</center></td>"
                    echo "<td><center>-</center></td>"
                    echo "</tr>"
                fi
            done < /var/run/dhcp_range_net.list
            echo "</table>"
            show_tab_footer
        fi

        if [ ! -f /etc/dnsmasq.d/dhcp_hosts_NET*.*conf -a ! -f /tmp/dhcp_hosts_NET*.add -a ! -f /etc/dnsmasq.d/dhcp_range_NET*.conf ]
        then
            show_info "$_STAT_DHCP_INFO" "$_STAT_DHCP_ERR_NOHOSTS"
        else
            echo "<br />"
            show_tab_header "$_STAT_DHCP_HOSTS_HOSTS" "no"
            echo "<table class=\"normtable\">"
        
            while read netid
            do
                echo "<tr>"
                echo "<th><center>$_STAT_DHCP_NETWORK</center></th>"
                echo "<th><center>$_STAT_DHCP_HOSTNAME</center></th>"
                echo "<th><center>$_STAT_DHCP_HOSTS_ALIAS</center></th>"
                echo "<th><center>$_STAT_DHCP_IPADDR</center></th>"
                echo "<th><center>$_STAT_DHCP_MACADDR<br />1</center></th>"
                echo "<th><center>$_STAT_DHCP_MACADDR<br />2</center></th>"
                echo "<th><center>$_STAT_DHCP_LEASETIME<br />[sec]</center></th>"
                echo "<th><center>$_STAT_DHCP_HOSTS_ACTION</center></th>"
                echo "</tr>"
                if [ -f /etc/dnsmasq.d/dhcp_hosts_NET${netid}.conf ]
                then
                    anzahl=0
                    while read line
                    do
                        anzahl=`expr $anzahl "+" 1`
                    done < /etc/dnsmasq.d/dhcp_hosts_NET${netid}.conf
                    while read line
                    do
                        split_dhcp_host_data
                        echo "<tr>"
                        if [ $anzahl -ne 0 ]
                        then
                            echo "<td rowspan='$anzahl'><center>IP_NET_$netid</center></td>"
                            anzahl=0
                        fi
                        aliases=""
                        for line in `grep ",$hostname\." /etc/dnsmasq.d/cname.conf`
                        do
                            aliases="$aliases`echo $line| cut -d "=" -f2|cut -d "," -f 1`<br />"
                        done
                        if [ "x$aliases" = "x" ]
                        then
                            aliases="-"
                        fi
                        echo "<td><center>$hostname</center></td>"
                        echo "<td><center>$aliases</center></td>"
                        echo "<td><center>$ipaddr</center></td>"
                        echo "<td><center>$mac</center></td>"
                        echo "<td><center>$mac2</center></td>"
                        echo "<td><center>$lease_time</center></td>"
                        echo "<td><center>-</center></td></tr>"
                    done < /etc/dnsmasq.d/dhcp_hosts_NET${netid}.conf
                fi
            
                if [ -f /etc/dnsmasq.d/dhcp_hosts_NET${netid}.wwwconf ]
                then
                    anzahl=0
                    while read line
                    do
                        anzahl=`expr $anzahl "+" 1`
                    done < /etc/dnsmasq.d/dhcp_hosts_NET${netid}.wwwconf
                    while read line
                    do
                        split_dhcp_host_data
                        echo "<tr style=\"background-color: #CFD;\">"
                        if [ $anzahl -ne 0 ]
                        then
                            echo "<td rowspan='$anzahl'><center>IP_NET_$netid<br />(added by webconf)</center></td>"
                            anzahl=0
                        fi
                        echo "<td><center>$hostname</center></td>"
                        echo "<td></td>"
                        echo "<td><center>$ipaddr</center></td>"
                        echo "<td><center>$mac</center></td>"
                        echo "<td><center>-</center></td>"
                        echo "<td><center>$lease_time</center></td>"
                        echo "<td><center><a href=\"$myname?action=delete&amp;hostname=$hostname&amp;net=$netid\"><img src=\"../img/delete.gif\"></a></center></td>"
                        echo "</tr>"
                    done < /etc/dnsmasq.d/dhcp_hosts_NET${netid}.wwwconf
                fi
                echo "<tr><td colspan='6'></td></tr>"
            
                if [ -f /tmp/dhcp_hosts_NET${netid}.add ]
                then
                    while read line
                    do
                        split_dhcp_host_data
                        echo "<form name=\"create_host\" action='"$myname"' method=\"get\">"
                        echo "<tr style=\"background-color: #DFDFDF;\">"
                        echo "<td><center>IP_NET_$netid<br />(add dynamic as static)</center>"
                        echo "<input type=\"hidden\" name=\"netid\" value=\"$netid\"></td>"
                        echo "<td><input style=\"margin-left:3px;\" type=\"text\" name=\"host\" size=\"15\" value=\"$hostname\"></td>"
                        echo "<td></td>"
                        echo "<td><input style=\"margin-left:3px;\" type=\"text\" name=\"ip1\" size=\"2\" value=\"\">"
                        echo ".&nbsp;<input style=\"margin-left:0px;\" type=\"text\" name=\"ip2\" size=\"2\" value=\"\">"
                        echo ".&nbsp;<input style=\"margin-left:0px;\" type=\"text\" name=\"ip3\" size=\"2\" value=\"\">"
                        echo ".&nbsp;<input style=\"margin-left:0px;\" type=\"text\" name=\"ip4\" size=\"2\" value=\"\"></td>"
                        echo "<td><input style=\"margin-left:3px;\" type=\"text\" name=\"mac\" size=\"15\" value=\"$mac\"></td>"
                        echo '<td></td>'
                        echo "<td><input style=\"margin-left:3px;\" type=\"text\" name=\"leasetime\" size=\"6\" value=\"$lease_time\"></td>"
                        echo "<td>&nbsp;&nbsp;"
                        echo '<button name="action" type="button " value="add" class="actions">'
                        echo "<img src=\"../img/add.gif \">"
                        echo "</button>"
                        echo "&nbsp;&nbsp;</td>"
                        echo "</tr>"
                        echo "</form>"
                    done < /tmp/dhcp_hosts_NET${netid}.add
                    rm /tmp/dhcp_hosts_NET${netid}.add 
                fi
                echo "<form name=\"create_host\" action='"$myname"' method=\"get\">"
                echo "<tr style=\"background-color: #DFDFDF;\">"
                echo "<td><center>IP_NET_$netid<br />(runtime add)</center>"
                echo "<input type=\"hidden\" name=\"netid\" value=\"$netid\"></td>"
                echo "<td><input style=\"margin-left:3px;\" type=\"text\" name=\"host\" size=\"15\" value=\"\"></td>"
                echo "<td></td>"
                echo "<td><input style=\"margin-left:3px;\" type=\"text\" name=\"ip1\" size=\"2\" value=\"\">"
                echo ".&nbsp;<input style=\"margin-left:0px;\" type=\"text\" name=\"ip2\" size=\"2\" value=\"\">"
                echo ".&nbsp;<input style=\"margin-left:0px;\" type=\"text\" name=\"ip3\" size=\"2\" value=\"\">"
                echo ".&nbsp;<input style=\"margin-left:0px;\" type=\"text\" name=\"ip4\" size=\"2\" value=\"\"></td>"
                echo "<td><input style=\"margin-left:3px;\" type=\"text\" name=\"mac\" size=\"15\" value=\"\"></td>"
                echo '<td></td>'
                echo "<td><input style=\"margin-left:3px;\" type=\"text\" name=\"leasetime\" size=\"6\" value=\"3600\"></td>"
                echo "<td>&nbsp;&nbsp;"
                echo '<button name="action" type="button " value="add" class="actions">'
                echo "<img src=\"../img/add.gif \">"
                echo "</button>"
                echo "&nbsp;&nbsp;</td>"
                echo "</tr>"
                echo "</form>"
                echo "<tr><td colspan='6'></td></tr>"
            done < /var/run/dhcp_range_net.list
            echo "</table>"
            show_tab_footer
            echo '<p><br /><b>'$_STAT_DHCP_INFO'</b><br/>'
            echo $_STAT_DHCP_INFOTEXT1 '<br />'
            echo $_STAT_DHCP_INFOTEXT2 '</p>'
            echo '<form action="'$myname'" method="GET">'
            echo '<input type="submit" value="Download" name="action" class="actions"></form>'
            echo '<br /><br />&nbsp;'
        fi
        show_html_footer
        ;;
esac
