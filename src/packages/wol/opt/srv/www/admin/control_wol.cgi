#!/bin/sh

# get main helper functions
. /srv/www/include/cgi-helper

check_rights "wol" "view"

_rshort=5 # short reload (5)
_rurl=$myname # reload-url

case $FORM_wolhost in
    "")
        show_html_header "Wake On LAN" "cssfile=status"
        show_tab_header "$_WOL_tab_header" no
    
        cat <<EOF
<table class="normtable"><tr>
<th align="center">&nbsp;$_WOL_tab_host&nbsp;</th>
<th align="center">&nbsp;$_WOL_tab_ip&nbsp;</th>
<th align="center">&nbsp;$_WOL_tab_mac&nbsp;</th>
<th align="center">&nbsp;$_WOL_tab_iface&nbsp;</th>
<th align="center">&nbsp;$_WOL_tab_state&nbsp;</th>
<th align="center">&nbsp;WOL&nbsp;(dst=mac)</th>
<th align="center">&nbsp;WOL&nbsp;(dst=bcast)</th></tr>
EOF
        sed -e "s/\./ /g" /var/run/wol/hosts.mac | sort -k7 -k8n -k2n -k3n -k4n -k5n | while read mac ip1 ip2 ip3 ip4  host iface1 iface2
        do
            ip=$ip1.$ip2.$ip3.$ip4
                      
	    if [ -z "$iface2" ]
	    then
	        iface=$iface1
	    else
	        iface=$iface1.$iface2
	    fi
                        
            if cat /proc/net/arp | grep "$ip " | grep "0x2 " > /dev/null
            then
                state=online
            else
                state=offline
            fi
            echo "<tr class=\"$state\"><td>&nbsp;$host&nbsp;</td><td>&nbsp;$ip&nbsp;</td><td>&nbsp;$mac&nbsp;</td><td>&nbsp;$iface&nbsp;</td>"
            echo "<td align=\"center\"><img src=\"/img/"$state".png\" width=\"16\" height=\"16\" class=\"tooltip::${_WOL_tab_state};$state\" alt=\"$state\"></td>"
            echo "<td align=\"center\"><a href=\"control_wol.cgi?wolhost=$host&bcast=no\"><img src=\"/img/execute.png\" width=\"16\" height=\"16\" class=\"tooltip::Host $host;$_WOL_execute\" alt=\"$_WOL_execute\"></a></td>"
            echo "<td align=\"center\"><a href=\"control_wol.cgi?wolhost=$host&bcast=yes\"><img src=\"/img/execute.png\" width=\"16\" height=\"16\" class=\"tooltip::Host $host;$_WOL_execute\" alt=\"$_WOL_execute\"></a></td>"
            echo "</tr>"
        done

        echo "</table><br /> $_WOL_note<br />&nbsp;"
        show_tab_footer
        ;;
    *)
        show_html_header "Wake On LAN" "refresh=$_rshort;url=$_rurl"
        if [ "$FORM_bcast" = "yes" ]
        then
            /usr/local/bin/wol.sh $FORM_wolhost -b
            show_info "$_WOL_done_info'$FORM_wolhost' (bcast)" "$_WOL_done_message<a href=$_rurl>$_WOL_done_url</a>."
        else
            /usr/local/bin/wol.sh $FORM_wolhost
            show_info "$_WOL_done_info'$FORM_wolhost' (mac)" "$_WOL_done_message<a href=$_rurl>$_WOL_done_url</a>."
        fi
        ;;
esac

show_html_footer
