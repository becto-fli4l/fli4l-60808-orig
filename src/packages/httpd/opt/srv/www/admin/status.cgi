#!/bin/sh
#------------------------------------------------------------------------------
# status.cgi - Display Status of ISDN & DSL
#
# Creation:     17.01.2001  tg
# Last Update:  $Id$
#
#------------------------------------------------------------------------------
# get main helper functions
# set_debug=yes

. /srv/www/include/cgi-helper

if grep -q "^OPT_IPV6='yes'$" /etc/rc.cfg && [ -f /srv/www/include/cgi-helper-ip6 ]
then
    . /srv/www/include/cgi-helper-ip6
    IPV6='yes'
fi

# Functions
show_connection_switch ()
{
    # Check for channel-bundling
    route=`imond-send "route" | sed 's/OK //; s/.$//'`
    links=`imond-send "links $route" | sed 's/OK //; s/.$//'`

    # Circuits
    circuits=`imond-send "circuits" | sed 's/.$//; s/OK //'`
    # check if a circuit is defined
    case $circuits in
        0);;
        *)
            comm=''
            for i in `seq 1 $circuits`
            do
              comm=`if [ "$comm" != "" ]; then echo "$comm"; fi; echo "circuit $i"`
            done
            circuits=`imond-send "$comm" |
              grep -n 'OK' | sed 's/^/<li><a href="'$myname'?action=circuit\&amp;circuit=/; s/:OK /">/;
              s/.$/<\/a><\/li>/'`
            circuits="<li><a href=\"$myname?action=circuit&amp;circuit=0\">Automatic (LCR)</a></li>
            $circuits"
            aktcirc=$route
            circuits=`echo "$circuits" | sed '/circuit='$aktcirc'/ s/<li/<li id="cselected"/'`
            echo "<ul class=\"circuits\"><li>$_STAT_circuits:</li>$circuits</ul>"

            # Dialmode
            dialmode="<li><a href=\"$myname?action=dialmode&amp;dialmode=auto\">$_STAT_dm_auto</a></li>
              <li><a href=\"$myname?action=dialmode&amp;dialmode=manual\">$_STAT_dm_manual</a></li>
              <li><a href=\"$myname?action=dialmode&amp;dialmode=off\">$_STAT_dm_off</a></li>"
            aktdialm=`imond-send "dialmode" | sed 's/OK //; s/.$//'`
            dialmode=`echo "$dialmode" | sed '/dialmode='$aktdialm'/ s/<li/<li id="dselected"/'`
            echo "<ul class=\"dialmode\"><li>$_STAT_dialmode:</li>$dialmode</ul>"

            # Dial/ Hangup
            echo '<form action="'$myname'" method="get">'
            echo -n '<input id="dial" type="submit"'
            case $onl in
                1) echo -n ' value="'$_STAT_hangup'" name="action" ' ;;
                *) echo -n ' value="'$_STAT_dial'" name="action"' ;;
            esac
            echo ' class="actions">'

            # Channel-Bundling?
            case $links in
                0|ERR) ;;
                *)
                    echo '<br>'
                    echo -n '<input type="submit"'
                    case "$links" in
                        1) echo -n ' value="'$_STAT_addlink'" name="action"' ;;
                        2) echo -n ' value="'$_STAT_remlink'" name="action"' ;;
                    esac
                    echo 'class="actions">'
                ;;
            esac
            echo '</form>'
            echo '<br>'
        ;;
    esac
}

SEC_ACTION='view'

case $FORM_action in
    "")
        headtitle="$_STAT_state"
    ;;
    *)
        ref="refresh=1;url=$myname"
        case $FORM_action in
            dialmode*)
                SEC_ACTION='dialmode'
            ;;
            circuit*)
                SEC_ACTION='circuit'
            ;;
            "$_STAT_dial"|"$_STAT_hangup")
                SEC_ACTION='dial'
                eval 'headtitle=$_'$FORM_action
            ;;
            "$_STAT_addlink"|"$_STAT_remlink") SEC_ACTION='link'
                eval 'headtitle=$_'$FORM_action
            ;;
            overview*)
                ref=""
            ;;
        esac
    ;;
esac

# Security
check_rights "status" "$SEC_ACTION"
show_html_header "$headtitle" "${ref}"
case "$FORM_action" in
    "$_STAT_hangup")
        show_info "" "$_STAT_hangingup ... `imond-send "hangup"`"
    ;;
    "$_STAT_dial")
        show_info "" "$_STAT_dialing ... `imond-send "dial"`"
    ;;
    "$_STAT_addlink")
        route=`imond-send "route" | sed 's/OK //; s/.$//'`
        show_info "" "$_STAT_addinglink ... `imond-send "addlink $route"`"
    ;;
    "$_STAT_remlink")
        route=`imond-send "route" | sed 's/OK //; s/.$//'`
        show_info "" "$_STAT_removinglink ... `imond-send "removelink $route"`"
    ;;

    circuit)
        show_info "" "$_STAT_changingcirc $(echo "$FORM_circuit" | htmlspecialchars) ... `imond-send "route $FORM_circuit"`"
    ;;
    dialmode)
        show_info "" "$_STAT_changingdialmode \"$(echo "$FORM_dialmode" | htmlspecialchars)\" ...`imond-send "dialmode $FORM_dialmode"`"
    ;;
esac

#---------------------------------------------------------------------------------------------
#------------------------- Uptime ------------------------------------------------------------
#---------------------------------------------------------------------------------------------
show_tab_header "$_STAT_uptime" no

uptime=`cat /proc/uptime | cut -f1 -d"." `
days=`expr $uptime / 86400`
hours=`expr $uptime % 86400 / 3600 `
minutes=`expr $uptime % 3600 / 60`
#seconds=`expr $uptime % 60`

L_STAT_days=`expr length $_STAT_days`
L_STAT_hours=`expr length $_STAT_hours`
L_MN_and=`expr length $_MN_and`
L_STAT_minutes=`expr length $_STAT_minutes`
size=`expr $L_STAT_days + $L_STAT_hours + $L_MN_and + $L_STAT_minutes + 16`

uptime="$days $_STAT_days, $hours $_STAT_hours $_MN_and $minutes $_STAT_minutes."

echo "<table class=\"normtable\"><tr><th>$HOSTNAME $_STAT_runningsince:</th></tr>"
echo "<tr><td><form action=\"$myname\" method=\"get\" class=\"uptimetext\"><input type=\"Text\" name=\"uptime\" id=\"uptime\" value=\"$uptime\" size=\"$size\" readonly></form></td></tr>"
echo '</table>'
show_tab_footer

echo "<br>"

#---------------------------------------------------------------------------------------------
#------------------------- CPU-USAGE ---------------------------------------------------------
#---------------------------------------------------------------------------------------------

cpu1m=`cat /proc/loadavg | cut -d " " -f 1`
cpu5m=`cat /proc/loadavg | cut -d " " -f 2`
cpu15m=`cat /proc/loadavg | cut -d " " -f 3`

show_tab_header "$_STAT_cpuusage" no
echo "<table class=\"normtable\">"
echo "<tr><th width=50>&nbsp;1min</th><th width=50>&nbsp;5min</th><th width=50>&nbsp;15min</th></tr>"
echo "<tr><td>$cpu1m</td><td>$cpu5m</td><td>$cpu15m</td></tr>"
echo "</table>"
show_tab_footer
echo "<br>"

if have_imond
then
    set -- `imond-send "channels" "pppoe" "cpu" | sed 's/\(OK\|ERR\) //; s/.$//'`
    chan="$1"
    pppoe="$2"
    usage="$3"
fi

#---------------------------------------------------------------------------------------------
#------------------------- Last Call ---------------------------------------------------------
#---------------------------------------------------------------------------------------------

# Check if telmond is running
if ps | grep -q [t]elmond
then
    set -- `telmond-send "" | sed 's/.$//'`
    case $1 in
        "") ;;
        *)
            LASTTIME="$1 $2"
            LASTFROM="$3"
            LASTTO="$4"

            # Find names in phonebook
            NAME=`grep "^$LASTFROM=" /etc/phonebook | cut -d"=" -f 2- | cut -d"," -f 1`
            if [ "$NAME" != "" ]; then LASTFROM="$NAME"; fi
            NAME=`grep "^$LASTTO=" /etc/phonebook | cut -d"=" -f 2- | cut -d"," -f 1`
            if [ "$NAME" != "" ]; then LASTTO="$NAME"; fi

            show_tab_header "$_STAT_lastcall" no
            echo '<table class="normtable">'
            echo '<tr><th rowspan="2">'$_STAT_lastcall':</th>'
            echo '<th>'$_STAT_from'</th><th>'$_STAT_to'</th>'
            echo '<th>'$_MN_date' / '$_MN_time'</th></tr><tr>'
            echo "<td>$LASTFROM</td><td>$LASTTO</td>"
            echo "<td>$LASTTIME</td></tr></table>"
            show_tab_footer
            echo "<br>"
        ;;
    esac
fi

#---------------------------------------------------------------------------------------------
#------------------------- Connection-Status -------------------------------------------------
#---------------------------------------------------------------------------------------------

if have_imond
then
    onl=0
    devs=''
    for i in `seq 1 $chan`
    do
      devs="$devs $i"
    done
    case $pppoe in
        1) devs="pppoe $devs" ;;
    esac

    # check if there are devices to display
    case $devs in
        "") ;;
        *)
            show_tab_header "$_STAT_connectionstate" no
            echo '<table class="normtable">'
            echo "<tr><th>$_STAT_state</th><th>$_STAT_name</th><th>$_STAT_direction</th><th>$_STAT_ip</th>
                  <th>$_STAT_ibytes</th><th>$_STAT_obytes</th><th>$_STAT_onltime</th></tr>"

            for i in $devs
            do
                cname=
                cdir=
                cip=
                conltime=
                cirate=
                corate=
                cibytes=
                cobytes=
                ctime=
                chtime=
                charge=
                conninfo=`imond-send "status $i" "phone $i" "inout $i" "ip $i" "online-time $i" "rate $i" "quantity $i" "time $i" "chargetime $i" "charge $i" | sed 's/OK //; s/.$//'`
                set -- $conninfo
                case $1 in
                    Off*)
                        cimg=rotaus
                        #cibytes=`bytes2read $7 $8`B
                        #cobytes=`bytes2read $9 ${10}`B
                        ctime=${12}
                        chtime=${13}
                        charge=${14}
                    ;;
                    Call*)
                        cimg=gelbaus
                        cname=$2
                        #cibytes=`bytes2read $7 $8`B
                        #cobytes=`bytes2read $9 ${10}`B
                        ctime=${12}
                        chtime=${13}
                        charge=${14}
                    ;;
                    On*)
                        cimg=gruenaus
                        cname=$2
                        eval cdir='$_STAT_'$3
                        case $# in 14) cip=$4; shift 1 ;; esac
                        conltime=$4
                        cirate=`bytes2read 0 $5`B/s
                        corate=`bytes2read 0 $6`B/s
                        cibytes=`bytes2read $7 $8`B
                        cobytes=`bytes2read $9 ${10}`B
                        ctime=${11}
                        chtime=${12}
                        charge=${13}
                        onl=1
                    ;;
                esac
                form='<form action="$myname" method="get" class="connectiontext">'
                echo '<tr>'
                echo '  <td><img id="cimg'$i'" src="../img/'$cimg'.gif" width="16" height="16" alt=""></td>'
                echo '  <td>'$form'<input type="Text" id="cname'$i'" value="'$cname'" size="15" readonly></form></td>'
                echo '  <td>'$form'<input type="Text" id="cdir'$i'" value="'$cdir'" size="10" readonly></form></td>'
                echo '  <td>'$form'<input type="Text" id="cip'$i'" value="'$cip'" size="15" readonly></form></td>'
                #echo '  <td>'$form'<input type="Text" id="cirate'$i'" value="'$cirate'" size="6" readonly></form></td>'
                #echo '  <td>'$form'<input type="Text" id="corate'$i'" value="'$corate'" size="6" readonly></form></td>'
                echo '  <td>'$form'<input type="Text" id="cibytes'$i'" value="'$cibytes'" size="9" readonly></form></td>'
                echo '  <td>'$form'<input type="Text" id="cobytes'$i'" value="'$cobytes'" size="9" readonly></form></td>'
                echo '  <td>'$form'<input type="Text" id="conltime'$i'" value="'$conltime'" size="8" readonly></form></td>'
                #echo '  <td>'$form'<input type="Text" id="ctime'$i'" value="'$ctime'" size="6" readonly></form></td>'
                #echo '  <td>'$form'<input type="Text" id="chtime'$i'" value="'$chtime'" size="6" readonly></form></td>'
                #echo '  <td>'$form'<input type="Text" id="charge'$i'" value="'$charge'" size="6" readonly></form></td>'
                echo '</tr>'
            done

            echo "</table>"
            show_connection_switch
            show_tab_footer
            echo "<br>"
        ;;
    esac
fi

#---------------------------------------------------------------------------------------------
#------------------------- DNS-Forwarder -----------------------------------------------------
#---------------------------------------------------------------------------------------------
if [ -f /etc/resolv.dnsmasq ]
then
    show_tab_header "$_STAT_dnsforws" no
    echo '<table class="normtable"><tr><th>IP</th><th>'$_STAT_dnsname'</th></tr>'
    resolver=`grep nameserver /etc/resolv.dnsmasq | cut -d " " -f 2`
    for i in $resolver
    do
        # lookup dns name of resolver, only if router is online
        if fli4lctrl status >/dev/null 2>&1
        then
            get_dns_name $i
        else
            res="n/a"
        fi
        echo "<tr><td>$i</td><td>$res</td></tr>"
    done
    echo "</table>"
    show_tab_footer
    echo '<br>'
else
    # we use dns-forwarders with special ports
    if [ -f /etc/dnsmasq.d/dns_resolver.conf ]
    then
        show_tab_header "$_STAT_dnsforws" no
        echo '<table class="normtable"><tr><th>IP</th><th>'$_STAT_dnsname'</th><th>Port</th></tr>'
        while read line
        do
            eval tmpstr=`echo "$line" | cut -b 8- `
            eval tmpip=`echo "$tmpstr" | cut -f 1 -d "#"`
            eval tmpport=`echo "$tmpstr" | cut -f 2 -d "#"`
            # if we have no port separated by #
            if [ "$tmpip" = "$tmpport" ]
            then
                tmpport="53"
            fi
            if [ "$tmpip" != "" ]
            then
                if fli4lctrl status >/dev/null 2>&1
                then
                    get_dns_name $tmpip
                else
                    res="n/a"
                fi
                echo "<tr><td>$tmpip</td><td>$res</td><td>$tmpport</td></tr>"
            fi
        done < /etc/dnsmasq.d/dns_resolver.conf
        echo "</table>"
        show_tab_footer
        echo '<br>'
    fi
fi



#---------------------------------------------------------------------------------------------
#------------------------- ARP Table -------------------------------------------------
#---------------------------------------------------------------------------------------------
show_tab_header "$_STAT_arptable" no
tooltipTitle='Interface'

if [ ${IPV6} == 'yes' ]
then
    echo '<table class="normtable"><tr><th>'$_STAT_state'</th><th>'$_STAT_name'</th><th>IPv4</th><th>IPv6</th><th>MAC</th><th>Interface</th></tr>'
else
    echo '<table class="normtable"><tr><th>'$_STAT_state'</th><th>'$_STAT_name'</th><th>IPv4</th><th>MAC</th><th>Interface</th></tr>'
fi

# TEMPORARILY REVERTED: only works for VLAN interfaces such as eth0.1 but fails for normal interfaces like eth0
# For dual stack (ipv4 + ipv6) configurations we get for each ipv4 client address the associated ipv4 addresses. 
# Usually there's several ipv6 addresses per client
# This will not yet work for ipv6 only setups but I assume those are very rare anyway. Can be extended later
#
# Given output to parse:
#    192.168.2.2 dev eth0.1 lladdr 00:1f:28:e4:4d:a0 STALE
#    192.168.2.26 dev eth0.1 lladdr c6:bc:e8:c2:58:25 DELAY
#    192.168.200.37 dev eth0.30 lladdr c8:2b:96:53:84:05 REACHABLE
#    192.168.178.26 dev eth0.100  FAILED
# 1. Replace dots with spaces (in ipv4 addresses and interfaces)
#    192 168 2 2 dev eth 0 1 lladdr 00:1f:28:e4:4d:a0 STALE
#    192 168 2 26 dev eth 0 1 lladdr c6:bc:e8:c2:58:25 DELAY
#    192 168 200 37 dev eth 0 30 lladdr c8:2b:96:53:84:05 REACHABLE
#    192 168 178 26 dev eth 0 100  FAILED
# 2. Now sort in the following order:
#    - alphabetic
#      - by network device name
#    - numeric
#      - by network device number
#      - VLAN ID (if existing)
#      - IP part A
#      - IP part B
#      - IP part C
#      - IP part D
# 3. Read values
#ip -4 n | sed -e "s/\./ /g" -e "s/\( [[:alpha:]]\{2,\}\)\([[:digit:]].* \)/\1 \2/g" -e "s/\( [[:alpha:]]\{2,\}\)\([[:digit:]].*$\)/\1 \2/g" | sort -k6g -k7n -k8n -k1n -k2n -k3n -k4n | while read ipA ipB ipC ipD devS devName devNo1 devNo2 fill mac status


# TEMPORARILY REENABLED: old version via /proc/net/arp works well for physical and VLAN interfaces
# Given output to parse:
#    "192.168.0.1      0x1         0x2         80:c6:ab:d8:9b:92     *        wlan0"
#    "192.168.42.111   0x1         0x2         00:24:8c:07:6e:01     *        eth1.1"
# 1. Replace dots with spaces
#    "192 168 0 1      0x1         0x2         80:c6:ab:d8:9b:92     *        wlan0"
#    "192 168 42 111   0x1         0x2         00:24:8c:07:6e:01     *        eth1 1"
# 2. Separate alphabetic characters from numeric characters followed by a space
#    with a space. This is necessary to be able to sort numeric by network interface
#    "192 168 0 1      0x1         0x2         80:c6:ab:d8:9b:92     *        wlan0"
#    "192 168 42 111   0x1         0x2         00:24:8c:07:6e:01     *        eth 1 1"
# 3. Separate alphabetic characters from numeric characters at end of line with
#    a space. This is necessary to be able to sort numeric by network interface
#    "192 168 0 1      0x1         0x2         80:c6:ab:d8:9b:92     *        wlan 0"
#    "192 168 42 111   0x1         0x2         00:24:8c:07:6e:01     *        eth 1 1"
# 4. Now sort in the following order:
#    "192 168 42 111   0x1         0x2         00:24:8c:07:6e:01     *        eth 1 1"
#    "192 168 0 1      0x1         0x2         80:c6:ab:d8:9b:92     *        wlan 0"
# 5. Exclude the headline by grep'ing "-v address"
# 6. Read values
sed -e "s/\./ /g" -e "s/\( [[:alpha:]]\{2,\}\)\([[:digit:]].* \)/\1 \2/g" -e "s/\( [[:alpha:]]\{2,\}\)\([[:digit:]].*$\)/\1 \2/g" /proc/net/arp | sort -k9 -k10n -k11n -k1n -k2n -k3n -k4n | grep -v address | while read ipA ipB ipC ipD hwtype flags mac foo devName devNo1 devNo2
do
    # Put IP values together
    ip=$ipA.$ipB.$ipC.$ipD

    # VLAN ID might be empty, so prevent the "."
    if [ -z "$devNo2" ]
    then
        dev=$devName$devNo1
    else
        dev=$devName$devNo1.$devNo2
    fi


    #case $status in
    #    REACHABLE|STALE|DELAY) status="online";;
    case $flags in
        0x2) status="online";;
        *) status="offline"; mac="-";;
    esac
    
    get_dns_name $ip

    if [ ${IPV6} == 'yes' ]
    then
        #get ipv6 addresses of this device
        #device might have several ipv6 in other subnets as well - thus also filter for device
        ip6=$(get_ip6_for_mac $mac $dev)
        echo "<tr class=\"$status\"><td>$status</td><td><a href=\"http://$res\">$res</a></td><td>$ip</td><td>$ip6</td><td>$mac</td><td>$dev</td></tr>"
    else
        echo "<tr class=\"$status\"><td>$status</td><td><a href=\"http://$res\">$res</a></td><td>$ip</td><td>$mac</td><td>$dev</td></tr>"
    fi
done

echo "</table>"
show_tab_footer
echo '<br>'

if have_imond
then
    echo '<div id="refresh">'
    #echo "$_MN_refresh ..."
    echo '<iframe src="status_refresh.cgi" class="HiddenFrame">'
    echo '</iframe>'
    echo '</div>'
fi
show_html_footer
