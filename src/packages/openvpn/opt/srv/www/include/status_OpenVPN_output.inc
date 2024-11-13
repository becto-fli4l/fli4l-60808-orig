#!/bin/sh
#------------------------------------------------------------------------------
#                                                                  __FLI4LVER__
# /srv/www/include/OpenVPN_output.inc
# Creation:     15.05.2005 hh
# Last Update:  $Id$
#------------------------------------------------------------------------------
[ "$gui_started" != "true" ] && exit 1 # must not be called standalone
#------- Output Functions -------------------------------------------------------
show_header ()
{ # show footer of html-file with header of main table
    # show back link if not on main page
    show_html_header "$1" "cssfile=status_OpenVPN"
    [ "$FORM_sec" = "status" ] || echo "<a href=\"$myname?showtext=$FORM_showtext\">$_MN_bck</a><br><br>"
    shift
    show_tab_header "$@"
}

show_footer ()
{ # show footer of html-file
    show_tab_footer
    case "$FORM_sec" in
        status)
            echo '<div id="refresh">'
            #echo "$_refresh ..."
            echo "<iframe src=\"status_OpenVPN_refresh.cgi?lang=$FORM_lang&amp;showtext=$FORM_showtext&amp;refresh=4\" class=\"HiddenFrame\">"
            echo '</iframe>'
            echo '</div>'
        ;;
    esac
    show_html_footer
}

show_lang ()
{ # show language-switch-link
    cd /usr/local/htdocs/openvpn/lang/
    ls *.cgi | cut -d. -f1 > /tmp/languages.$$
    echo "<div class=\"language\">"
    while read x;
    do
        [ "$FORM_lang" = "$x" ] || echo "<a href=\"$myname?lang=$x&amp;sec=$FORM_sec&amp;action=$FORM_action&amp;saction=$FORM_saction&amp;name=$FORM_name&amp;nrlog=$FORM_nrlog&amp;showtext=$FORM_showtext\">$x</a>&nbsp;"
    done < /tmp/languages.$$
    echo "</div>"
    rm -f /tmp/languages.$$
}

show_connections ()
{ # Show all connections with status

    # Get all ovpn names
    cd /etc/openvpn/
    
    # openvpn.names only exist when ovpn is config - not in expert-mode
    if [ ! -f openvpn.names ]
    then
         ls *.conf | cut -d. -f 1 > openvpn.names
    fi
    
    while read name
    do
        echo "$name `state_ovpn`" >> /tmp/ovpnnamestate.$$
    done < openvpn.names
    
    case $FORM_sort in
        normal)
            sort -f -k 1 /tmp/ovpnnamestate.$$ > /tmp/ovpnnames.$$
            ;;
        reverse)
            sort -fr -k 1 /tmp/ovpnnamestate.$$ > /tmp/ovpnnames.$$
            ;;
        state)
            sort -f -k 2 /tmp/ovpnnamestate.$$ > /tmp/ovpnnames.$$
            ;;
        *)
            cat /tmp/ovpnnamestate.$$ > /tmp/ovpnnames.$$
            ;;
    esac
    rm /tmp/ovpnnamestate.$$
    
    echo "<table class=\"normtable connection\"><tr><th><a href="$myname?sort=state">$_MT_stat</a></th><th>${_VPN_name}&nbsp;"
    echo "<a href="$myname?sort=normal"><img src=\"../img/snormal.gif\" alt=\" (sort normal)\"></a>&nbsp;"
    echo "<a href="$myname?sort=reverse"><img src=\"../img/sreverse.gif\" alt=\" (sort reverse)\"></a>"
    echo "</th><th>$_VPN_action</th></tr>"
    switch="odd"
    while read name state;
    do
        echo "<tr class=\"$switch\">"
        hold="hold"
        action="stop"
        case $state in
            STOPPED)
                symbol="stopped"
                action="start"
            ;;
            CONNECTED)
                symbol="running"
            ;;
            HOLD)
                symbol="noconn"
                hold="release"
            ;;
            *)
                symbol="noconn"
            ;;
        esac
        # show status-icons, name and action-icons
        echo "<td class=\"state\">`icon $symbol $state state-$name`<br />"
        case $FORM_showtext in
            yes)
                echo "<form action=\".\" method=\"get\" class=\"statetext\"><input type=\"text\" name=\"status\" class=\"$switch\" id=\"statetext-$name\" value=\"`eval echo '$_VPN_'$state`\" size=\"35\" readonly=\"readonly\"></form>"
            ;;
        esac
        echo "</td>"
        echo "<td><a href=\"$myname?lang=$FORM_lang&amp;sec=detail&amp;name=$name&amp;showtext=$FORM_showtext\" title=\"$_info\">$name</a></td><td class=\"icons\">"
        echo "<a class=\"imagelink\" href=\"$myname?lang=$FORM_lang&amp;sec=status&amp;action=$action&amp;name=$name&amp;showtext=$FORM_showtext\">`icon $action`</a>"
        if [ "$state" != "STOPPED" ]
        then
            echo "<a class=\"imagelink\" href=\"$myname?lang=$FORM_lang&amp;sec=status&amp;action=reload&amp;name=$name&amp;showtext=$FORM_showtext\">`icon reload`</a>"
            echo "<a class=\"imagelink\" href=\"$myname?lang=$FORM_lang&amp;sec=status&amp;action=$hold&amp;name=$name&amp;showtext=$FORM_showtext\">`icon $hold`</a>"
        fi

        echo '</td></tr>'
        # change row-color
        if [ "$switch" = "odd" ]
        then
            switch="even"
        else
            switch="odd"
        fi
    done < /tmp/ovpnnames.$$
    echo '</table>'
    rm -f /tmp/ovpnnames.$$
}

show_packetfilter ()
{ # Shows packetfilter-table
    case $2 in
        in)
            chain="Input"
        ;;
        fw)
            chain="Forward"
        ;;
        pi)
            chain="NAT Pre-Routing"
        ;;
        po)
            chain="NAT Post-Routing"
        ;;
    esac
    echo "<h2>$chain Chain:</h2>"
    echo "<table class=\"normtable chain\">"
    if /sbin/iptables -t $1 -nvL $2-ovpn-$3 > /dev/null
    then
        iptables -t $1 --line-numbers -nvL $2-ovpn-$3 | \
        while read line
        do
             case "$line" in
                 '') ;;
                 Chain*)
                     echo '    <tr><th colspan="11">'$line'</th></tr>'
                 ;;
                 num*)
                     echo "$line" | sed -e 's/^/<tr><th>/;s/[[:space:]]\+/<\/th><th>/g'
                     echo "</th><th>additional restrictions</th>" #<th>config source</th></tr>"
                 ;;
                 *)
                     set -f
                     set $line
                     def=`echo "$1 $2 $3 $4 $5 $6 $7 $8 $9 ${10}" | sed -e 's/^/<td>/;s/[[:space:]]\+/<\/td><td>/g;s/$/<\/td>/;s#<td>\(all\|[*]\|--\|0\.\0\.\0\.0/0\)</td>#<td>\&nbsp;</td>#g'`
                     target=$4
                     shift 10
                     restr=`echo "<td>$*</td>" | sed -e 's,/[*].*[*]/,,;s#<td></td>#<td>\&nbsp;</td>#g'`
                     src=`echo "$*" | sed -e 's#.*/[*]\(.*\)[*]/#\1#'`
                     echo "<tr title=\"$src\" class=\"$target\">$def $restr</tr>"
                     set +f
                 ;;
             esac
        done
    else
        echo "<tr><td colspan=\"10\">`eval echo '$_no_VPN_'$1'chain'`</td></tr>"
    fi
    echo "</table>"
}

show_packetfilter6 ()
{ # Shows packetfilter-table
    case $2 in
        in)
            chain="Input"
        ;;
        fw)
            chain="Forward"
        ;;
    esac
    echo "<h2>$chain Chain IPv6:</h2>"
    echo "<table class=\"normtable chain\">"
    if /sbin/ip6tables -t $1 -nvL $2-ovpn-$3 > /dev/null
    then
        ip6tables -t $1 --line-numbers -nvL $2-ovpn-$3 | \
        while read line
        do
             case "$line" in
                 '') ;;
                 Chain*)
                     echo '    <tr><th colspan="11">'$line'</th></tr>'
                 ;;
                 num*)
                     echo "$line" | sed -e 's/^/<tr><th>/;s/[[:space:]]\+/<\/th><th>/g'
                     echo "</th><th>additional restrictions</th>" #<th>config source</th></tr>"
                 ;;
                 *)
                     set -f
                     set -- $line
                     line="$1 $2 $3 $4 $5 --"
                     shift 5
                     line="$line $*"
                     set -- $line
                     def=`echo "$1 $2 $3 $4 $5 $6 $7 $8 $9 ${10}" | sed -e 's/^/<td>/;s/[[:space:]]\+/<\/td><td>/g;s/$/<\/td>/;s#<td>\(all\|[*]\|--\|0\.\0\.\0\.0/0\)</td>#<td>\&nbsp;</td>#g'`
                      target=$4
                     shift 10
                     restr=`echo "<td>$*</td>" | sed -e 's,/[*].*[*]/,,;s#<td></td>#<td>\&nbsp;</td>#g'`
                     echo "<tr title=\"$comment\" class=\"$target\">$def $restr</tr>"
                     set +f
                 ;;
             esac
        done
    else
        echo "<tr><td colspan=\"10\">`eval echo '$_no_VPN_'$1'chain'`</td></tr>"
    fi
    echo "</table>"
}

show_bridge ()
{ # Show bridge conig of current connection
    echo "<h2>$_VPN_head_bridge:</h2>"
    echo "<table class=\"normtable bridge\">"
    brctl show | sed -r 's/ /_/g;s/[[:space:]]+/~/g;s/$/<\/td><\/tr>/;s/^~/<\/td><td><\/td><td><\/td><td>/;\
                         s/~/<\/td><td>/g;s/^/<tr><td>/;s/_/ /g'
    echo "</table>"
}

show_stats ()
{ # Show table with statistics of current connection
    echo "<h2>$_VPN_head_stats:</h2>"
    case `state_ovpn` in
        HOLD|STOPPED)
            # no statistics available if connection is hold
            eval echo '$_VPN_'`state_ovpn`'.\<br\>'
            echo $_VPN_nostats
        ;;
        *)
            echo "<table class=\"normtable stats\">"
            echo '<tr><th>'$_VPN_what'</th><th colspan="2">'$_VPN_amount'</th></tr>'
            mkdir -p /tmp/ovpn-webgui.$$
            cd /tmp/ovpn-webgui.$$
            ocontrol "status" | grep -v -e "STATISTICS" -e "Updated" > a
            cat a | cut -d, -f1 | sed 's/ bytes//' > b
            cat a | cut -d, -f2 > c
            exec 4<b 5<c
            while read x <&4
            do
              read y <&5
              echo "<tr><td>$x</td><td class=\"number\">`cnum $y | sed '/[^B]$/s/$/<\/td><td>bytes/;s/ /<\/td><td>/'`</td></tr>"
            done
            rm -rf /tmp/ovpn-webgui.$$
            #ocontrol "status" | sed "s°^°<tr><td>°;s°$°</td></tr>°;s°,°</td><td>°" | grep -v "STATISTICS"
            echo "</table>"
        ;;
    esac
}

show_config ()
{ # Show configuration of current connection
    echo "<h2>$_VPN_head_config:</h2>"
    echo "<form action=\"$myname\" method=\"get\">"
    echo "<textarea class=\"itext\" cols=\"60\" rows=\"20\" name=\"config\" readonly>"
    cat /etc/openvpn/$name.conf
    echo "</textarea>"
    echo "<input type=\"hidden\" name=\"lang\" value=\"$FORM_lang\">"
    echo "<input type=\"hidden\" name=\"sec\" value=\"$FORM_sec\">"
    echo "<input type=\"hidden\" name=\"action\" value=\"$FORM_action\">"
    echo "<input type=\"hidden\" name=\"name\" value=\"$FORM_name\">"
    echo "</form>"
    echo "<br />"
}

show_logfile ()
{ # Show log of current connection
    case $1 in
    debug)
        mkdir -p /tmp/ovpn-webgui.$$
        cd /tmp/ovpn-webgui.$$
        echo $$ > /var/lock/ovpn-debug.lck
        start_ovpn debug | sed -r -e 's°[[:space:]]+° °g' > a &
        sleep 2
        stop_ovpn debug
        rm -f /var/lock/ovpn-debug.lck
        cat a | cut -d" " -f4 > b
        cat a | cut -d" " -f7-99 > c
        exec 4<b 5<c
        while read x <&4
        do
            read y <&5
            echo "<tr><td>$x</td><td>$y</td></tr>"
        done
        rm -rf /tmp/ovpn-webgui.$$
        echo "</table>"
    ;;
    *)
        mkdir -p /tmp/ovpn-webgui.$$
        cd /tmp/ovpn-webgui.$$
        ocontrol "log $1" | grep -v "HOLD" > a
        cat a | cut -d, -f1 > b
        cat a | cut -d, -f2 > c
        cat a | cut -d, -f3-99 | sed "s°$°</td></tr>°" > d
        exec 4<b 5<c 6<d
        while read x <&4
        do
            read y <&5
            read z <&6
            case x$y in
                x) ;;
                *) y=" class=\"$y\"" ;;
            esac
            echo "<tr$y><td>`ctime $x "$_VPN_dateformat<\/td><td>$_VPN_timeformat"`</td><td>$z"
        done
        rm -rf /tmp/ovpn-webgui.$$
        echo "</table>"
    ;;
    esac
}

show_support ()
{ # Show all support infos for current connection
    echo "<div class=\"support\"><h2>$_VPN_head_supp:</h2>"
    echo "<h2>$_VPN_head_state:</h2>"
    eval echo '$_VPN_'`state_ovpn`
    echo "<h2>$_VPN_head_config:</h2>"
    sed "s°$°<br />°" /etc/openvpn/$name.conf
    echo "<br />"
    if cat /etc/openvpn/$name.conf | grep -q "^dev tun"
    then
        show_packet
    else
        show_bridge
    fi
    case $1 in
        debug)
            echo "<h2>$_VPN_head_log</h2>"
            echo "<table class=\"normtable log\"><tr><th>$_MN_time</th><th>$_VPN_notice</th></tr>"
            show_logfile $1
        ;;
        *)
            echo "<h2>$_VPN_head_log ($_MN_all $_MN_rows):</h2>"
            echo "<table class=\"normtable\" id=\"log\"><tr><th>$_MN_date</th><th>$_MN_time</th><th>$_VPN_notice</th></tr>"
            show_logfile $1
        ;;
    esac

    echo "</div>"
}

show_dsupp ()
{ # Show all support infos for current connection
    show_support debug
}

show_dlog ()
{ # Show all support infos for current connection
    echo "<h2>$_VPN_head_log</h2>"
    echo "<table class=\"normtable\" id=\"log\"><tr><th>$_MN_time</th><th>$_VPN_notice</th></tr>"
    show_logfile debug
}

show_packet ()
{ # Show packetfilter of current connection
    case `state_ovpn` in
        HOLD|STOPPED)
            # no statistics available if connection is hold
            eval echo '$_VPN_'`state_ovpn`'.\<br /\>'
            echo $_VPN_nopacket
        ;;
        *)
            show_packetfilter filter in $name
            show_packetfilter filter fw $name
            show_packetfilter nat pi $name
            show_packetfilter nat po $name
            if [ -n "`grep ^tun-ipv6 /etc/openvpn/$name.conf`" ]
            then
                show_packetfilter6 filter in $name
                show_packetfilter6 filter fw $name
            fi
        ;;
    esac
}

show_supp ()
{ # Show all Supportinfos
    show_support all
}

show_log ()
{ # Show log of current connection
    case "$FORM_nrlog" in
        all|$_MN_all|*[^0-9]*) n=$_MN_all ;;
        *[0-9]*) n=$FORM_nrlog ;;
    esac
    echo "<h2>$_VPN_head_log ($n $_MN_rows):</h2>"
    # Display form to change number of shown rows of logfile
    echo "<form action=\"$myname\" method=\"get\">"
    echo "<input type=\"hidden\" name=\"lang\" value=\"$FORM_lang\">"
    echo "<input type=\"hidden\" name=\"sec\" value=\"$FORM_sec\">"
    echo "<input type=\"hidden\" name=\"action\" value=\"$FORM_action\">"
    echo "<input type=\"hidden\" name=\"name\" value=\"$FORM_name\">"
    echo "<input class=\"itext\" type=\"text\" size=\"3\" maxlength=\"3\" name=\"nrlog\" value=\"$FORM_nrlog\">&nbsp;$_MN_rows&nbsp;&nbsp;"
    echo "<input class=\"sbutton\" type=\"submit\" value=\"$_MN_show\">"
    echo "</form>"
    echo "<table class=\"normtable\" id=\"log\"><tr><th>$_MN_date</th><th>$_MN_time</th><th>$_VPN_notice</th></tr>"
    show_logfile $FORM_nrlog
}


show_secret ()
{ # create secret
    [ -f /etc/openvpn/$keyfile -o -f /tmp/$keyfile ] || create_secret
    if [ -f /tmp/$keyfile ] && [ -f /etc/openvpn/$keyfile ] || [ -f /etc/openvpn/$keyfile.orig ]
    then
        rlink="<a href=\"$myname?lang=$FORM_lang&amp;sec=detail&amp;saction=restore&amp;action=secret&amp;name=$name&amp;showtext=$FORM_showtext\"><img src=\"../img/undo.gif\" width=\"16\" height=\"16\" alt=\"undo\" title=\"$_VPN_undo\"></a>"
    else
        rlink="<img src=\"../img/noundo.gif\" width=\"16\" height=\"16\" alt=\"undo\" title=\"$_VPN_undo\">"
    fi
    if [ -f /tmp/$keyfile ]
    then
        slink="<a href=\"$myname?lang=$FORM_lang&amp;sec=detail&amp;saction=save&amp;action=secret&amp;name=$name&amp;showtext=$FORM_showtext\"><img src=\"../img/save.gif\" width=\"16\" height=\"16\" alt=\"save\" title=\"$_VPN_save\"></a>"
        keyfile=/tmp/$keyfile
    else
        slink="<img src=\"../img/nosave.gif\" width=\"16\" height=\"16\" alt=\"save\" title=\"$_VPN_save\">"
        keyfile=/etc/openvpn/$keyfile
    fi
    echo "<h2>Secret: $keyfile</h2>"
    echo "<br />"
    echo "<a href=\"$myname?lang=$FORM_lang&amp;sec=detail&amp;saction=create&amp;action=secret&amp;name=$name&amp;showtext=$FORM_showtext\"><img src=\"../img/new.gif\" width=\"16\" height=\"16\" alt=\"new\" title=\"$_VPN_new\"></a>"
    echo $slink
    echo $rlink
    echo "<a href=\"$myname?lang=$FORM_lang&amp;sec=detail&amp;saction=download&amp;action=secret&amp;name=$name&amp;showtext=$FORM_showtext\"><img src=\"../img/download.gif\" width=\"16\" height=\"16\" alt=\"download\" title=\"$_MN_download\"></a>"
    echo "<form action=\".\" method=\"get\">"
    echo "<textarea class=\"itext\" cols=\"60\" rows=\"21\" name=\"config\" readonly>"
    cat $keyfile
    echo "</textarea>"
    echo "</form>"
}

