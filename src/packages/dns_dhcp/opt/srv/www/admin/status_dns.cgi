#!/bin/sh

#----------------------------------------------------------------------------
# /srv/www/admin/status_dns.cgi
#
# Creation:     LanSpezi 
# Last Update:  $Id$
#----------------------------------------------------------------------------

## functions

#main

. /srv/www/include/cgi-helper

if [ -f /tmp/template_dns.txt ]
then
    rm /tmp/template_dns.txt
fi

check_rights "status" "change"

case $FORM_action in
    Download)
        if [ -f /etc/dnsmasq.d/dns_forbidden.wwwconf ]
        then
            http_header download "ctype=text/plain;filename=dns_forbidden.txt"
            {
                echo "#"
                echo "# please copy needed data to your config/dns_dhcp.txt and replace x by correct numbers"
                echo "#"
                echo "# do not forget to correct DNS_FORBIDDEN_N"
                echo "#"
            } >> /tmp/template_dns.txt
            while read line
            do
                dom=`echo "$line" | cut -d "/" -f2`
                echo "DNS_FORBIDDEN_x='$dom'" >> /tmp/template_dns.txt
            done < /etc/dnsmasq.d/dns_forbidden.wwwconf
    
            cat /tmp/template_dns.txt
        else
            ref="refresh=5;url=$myname?action=view"
            show_html_header "$_STAT_DNS_MAINTITLE" "${ref}"
            show_info "DNS_FORBIDDEN" "$_STAT_DNS_DOWNLOAD"
            show_html_footer
        fi
    ;;
    add)
        ref="refresh=10;url=$myname?action=view"
        show_html_header "$_STAT_DNS_MAINTITLE " "${ref}"
        if [ "x$FORM_domain" = "x" ]
        then
            show_error "DNS_FORBIDDEN" "$_STAT_DNS_DOMAIN_NO"
        else
            temp=`echo "$FORM_domain" | grep -E "^([a-zA-Z0-9]([a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])?\.)+[a-zA-Z]{2,6}$"`
            if [ "x${temp}" = "x" ]
            then
                show_error "DNS_FORBIDDEN" "$FORM_domain $_STAT_DNS_DOMAIN_NOTVALID"
            else
                if grep -q "/${FORM_domain}/" /etc/dnsmasq.d/dns_forbidden.wwwconf /etc/dnsmasq.d/dns_forbidden.conf
                then
                show_error "DNS_FORBIDDEN" "$FORM_domain $_STAT_DNS_DOMAIN_DOUBLE"
                else
                    echo "local=/${FORM_domain}/" >> /etc/dnsmasq.d/dns_forbidden.wwwconf
                    show_info "DNS_FORBIDDEN" "$FORM_domain $_STAT_DNS_DOMAIN_ADDED"
                    echo "restart" > /tmp/reload_dnsmasq.wwwflag
                fi
            fi
        fi
        show_html_footer
    ;;

    delete)
        ref="refresh=10;url=$myname?action=view"
        show_html_header "$_STAT_DNS_MAINTITLE " "${ref}"
        grep -v "/${FORM_domain}/" /etc/dnsmasq.d/dns_forbidden.wwwconf > /tmp/dns_forbidden.wwwconf
        cat /tmp/dns_forbidden.wwwconf > /etc/dnsmasq.d/dns_forbidden.wwwconf
        rm /tmp/dns_forbidden.wwwconf
        show_info "DNS_FORBIDDEN" "${FORM_domain} $_STAT_DNS_DOMAIN_REMOVED"
        echo "restart" > /tmp/reload_dnsmasq.wwwflag
        show_html_footer
    ;;

    enable)
        ref="refresh=5;url=$myname?action=view"
        show_html_header "$_STAT_DNS_MAINTITLE " "${ref}"
        sed -i "s?#DIS#local=/${FORM_domain}/?local=/${FORM_domain}/?g" /etc/dnsmasq.d/dns_forbidden.conf

        show_info "DNS_FORBIDDEN" "${FORM_domain} $_STAT_DNS_DOMAIN_ENABLED"
        echo "restart" > /tmp/reload_dnsmasq.wwwflag
        show_html_footer
    ;;

    disable)
        ref="refresh=5;url=$myname?action=view"
        show_html_header "$_STAT_DNS_MAINTITLE " "${ref}"
        sed -i "s?local=/${FORM_domain}/?#DIS#local=/${FORM_domain}/?g" /etc/dnsmasq.d/dns_forbidden.conf

        show_info "DNS_FORBIDDEN" "${FORM_domain} $_STAT_DNS_DOMAIN_DISABLED"
        echo "restart" > /tmp/reload_dnsmasq.wwwflag
        show_html_footer
    ;;

    *|view)
        if [ -f /tmp/reload_dnsmasq.wwwflag ]
        then
            ref="refresh=5;url=$myname?action=view"
            show_html_header "$_STAT_DNS_MAINTITLE" "${ref}"
            rm /tmp/reload_dnsmasq.wwwflag
            killall dnsmasq
            show_info "$_STAT_DNS_RELOAD_HEAD" "$_STAT_DNS_MSG_RELOAD"
            echo "<br />"
            sleep 2
            dnsmasq
        else
            show_html_header "$_STAT_DNS_MAINTITLE " "${ref}"
        fi

        echo "<ul>"
        echo "<a href=\"#forbidden\">DNS_FORBIDDEN <img src=\"/img/sreverse.gif\"></a>"
        echo "</ul>"

        for hfile in /etc/hosts.d/*
        do
            if grep -q -v "^#" ${hfile}
            then
                show_tab_header "DNS-Hosts (${hfile})" no
                echo "<table class=\"normtable\">"
                echo "<tr>"
                echo "<th><center>$_STAT_DNS_HOSTNAME (FQDN)</center></th>"
                echo "<th><center>$_STAT_DNS_HOSTNAME</center></th>"
                echo "<th><center>$_STAT_DNS_HOSTS_ALIAS</center></th>"
                echo "<th><center>$_STAT_DNS_IPADDR</center></th>"
                echo "</tr>"
                grep -v "^#" ${hfile} | while read line
                do
                    ipa=`echo "$line" | tr -s " " | cut -d " " -f1`
                    hnamed=`echo "$line" | tr -s " " | cut -d " " -f2`
                    hname=`echo "$line" | tr -s " " | cut -d " " -f3`
                    if [ "$hfile" = "/etc/hosts.d/hosts" ]
                    then
                        alias=`echo "$line" | tr -s " " | cut -d " " -f4-`
                    else
                        alias=""
                        for a in `grep -w ${hnamed} /etc/dnsmasq.d/cname.conf | cut -d "=" -f2 | cut -d "," -f1`
                        do
                            alias=`echo "${alias}&nbsp;&nbsp;${a}&nbsp;&nbsp;<br />"`
                        done
                    fi
                    echo "<tr>"
                    echo "<td>&nbsp;&nbsp;${hnamed}&nbsp;&nbsp;</td>"
                    echo "<td>&nbsp;&nbsp;${hname}&nbsp;&nbsp;</td>"
                    echo "<td>${alias}</td>"
                    echo "<td>&nbsp;&nbsp;${ipa}&nbsp;&nbsp;</td>"
                    echo "</tr>"
                done
                echo "</table>"
                show_tab_footer
                echo "<br />"
            fi
        done

        if [ -f /etc/dnsmasq.d/dns_redirect.conf ]
        then
            show_tab_header "DNS_REDIRECT" no
            echo "<table class=\"normtable\">"
            echo "<tr>"
            echo "<th><center>$_STAT_DNS_DOMAIN</center></th>"
            echo "<th><center>$_STAT_DNS_IPADDR</center></th>"
            echo "</tr>"
            while read line 
            do
                dom=`echo "$line" | cut -d "/" -f2`
                domip=`echo "$line" | cut -d "/" -f3`
                echo "<tr>"
                echo "<td>&nbsp;&nbsp;${dom}&nbsp;&nbsp;</td>"
                echo "<td>&nbsp;&nbsp;${domip}&nbsp;&nbsp;</td>"
                echo "</tr>"            
            done < /etc/dnsmasq.d/dns_redirect.conf
            echo "</table>"
            show_tab_footer
            echo "<br />"
        fi

        show_tab_header "DNS_FORBIDDEN" no
        echo "<table class=\"normtable\"><a name=\"forbidden\"></a>"
        echo "<tr>"
        echo "<th><center>$_STAT_DNS_DOMAIN</center></th>"
        echo "<th><center>Aktion</center></th>"
        echo "</tr>"
        if [ -f /etc/dnsmasq.d/dns_forbidden.conf ]
        then
            while read line 
            do
                dom=`echo "$line" | cut -d "/" -f2`
                echo "<tr>"
                if [ `echo "$line" | grep "#DIS#"` ]
                then
                    echo "<td style=\"background-color: #ff0000;\">&nbsp;&nbsp;${dom}&nbsp;&nbsp;</td>"
                    echo "<td><center><a href=\"$myname?action=enable&amp;domain=$dom\">enable</a></center></td>"
                else
                    echo "<td>&nbsp;&nbsp;${dom}&nbsp;&nbsp;</td>"
                    echo "<td><center><a href=\"$myname?action=disable&amp;domain=$dom\">disable</a></center></td>"
                fi
                echo "</tr>"               
            done < /etc/dnsmasq.d/dns_forbidden.conf
        fi
        if [ -f /etc/dnsmasq.d/dns_forbidden.wwwconf ]
        then
            echo "<tr><td colspan='2'></td></tr>"
            while read line
            do
                dom=`echo "$line" | cut -d "/" -f2`
                echo "<tr style=\"background-color: #DFDFDF;\">"
                echo "<td>&nbsp;&nbsp;${dom}&nbsp;&nbsp;</td>"
                echo "<td><center><a href=\"$myname?action=delete&amp;domain=$dom\"><img src=\"../img/delete.gif\"></a></center></td>"
            done < /etc/dnsmasq.d/dns_forbidden.wwwconf
        fi        
        echo "<tr><td colspan='2'></td></tr>"
        echo "<form name=\"add_dnsforbidden\" action='"$myname"' method=\"get\">"
        echo "<tr style=\"background-color: #DFDFDF;\">"
        echo "<td><input style=\"margin-left:3px;\" type=\"text\" name=\"domain\" size=\"45\" value=\"\"></td>"
        echo "<td>&nbsp;&nbsp;"
        echo '<button name="action" type="button " value="add" class="actions">'
        echo "<img src=\"../img/add.gif \">"
        echo "</button>"
        echo "&nbsp;&nbsp;</td>"
        echo "</tr>"
        echo "</form>"
        echo "<tr><td colspan='2'></td></tr>"
        echo "</table>"
        show_tab_footer
        echo '<p><br /><b>'$_STAT_DNS_INFO'</b><br/>'
        echo $_STAT_DNS_INFOTEXT1 '<br />'
        echo $_STAT_DNS_INFOTEXT2 '</p>'
        echo '<form action="'$myname'" method="GET">'
        echo '<input type="submit" value="Download" name="action" class="actions"></form>'
        echo '<br /><br />&nbsp;'
        show_html_footer
    ;;
esac
