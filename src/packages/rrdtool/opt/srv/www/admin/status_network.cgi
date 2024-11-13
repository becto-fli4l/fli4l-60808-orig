#!/bin/sh
#------------------------------------------------------------------------------
# status_network.cgi - Display Status of Network (rrdtool graph)
#
# Creation:     2009-09-11 Lanspezi
# Last Update:  $Id$
#
#------------------------------------------------------------------------------
# get main helper functions
# set_debug=yes

. /srv/www/include/cgi-helper

# Security
check_rights "status" "view"

show_html_header "Netzwerkstatus" "${ref}"

#-------------------------------------------------------------------------------
#----------- Network Graphs ----------------------------------------------------
#-------------------------------------------------------------------------------

if [ -e /srv/www/include/rrd-status_network.inc ]
then
    . /srv/www/include/rrd-common.inc 
    . /srv/www/include/rrd-status_network.inc
    . /etc/boot.d/netalias.inc

    : ${FORM_rrd_graphtime_eth0=$rrd_default_graphtime}
    
    if [ "$FORM_section" = "" ]
    then
        if [ -f /var/run/network_section ]
        then
            FORM_section=$(cat /var/run/network_section)
            rm /var/run/network_section
        fi
    fi

    : ${FORM_section:=LAN}

    #INTERFACES=`ip link show | sed -n -e 's/^[0-9]\+:\([[:space:]a-z0-9.]*\).*/\1/p' | sort`
    INTERFACES=`ip link show | sed -n -e 's/^[0-9]\+:\([-[:space:]a-z0-9.]*\).*/\1/p' | sort`
    INTERFACES_LAN=`echo "$INTERFACES" | grep eth | grep -v "\."`
    INTERFACES_VLAN=`echo "$INTERFACES" | grep "\."`
    INTERFACES_BR=`echo "$INTERFACES" | grep br | grep -v "\."`
    INTERFACES_WLAN=`echo "$INTERFACES" | grep wlan | grep -v "\."`
    INTERFACES_BOND=`echo "$INTERFACES" | grep bond | grep -v "\."`
    INTERFACES_VPN=`echo "$INTERFACES" | egrep "tun|tap|wg"`
    INTERFACES_IMQ=`echo "$INTERFACES" | egrep "imq0|imq1" | egrep -v "imq1."`
    INTERFACES_OTHER=`echo "$INTERFACES" | egrep -v "eth|br|tun|tap|wlan|bond|imq|wg"`

    tab_section=""
    
    for s in LAN VLAN BR BOND VPN WLAN IMQ OTHER
    do
        eval SEC=\$'INTERFACES_'$s
        if [ "x$SEC" != "x" ]
        then
            if [ "$FORM_section" = "$s" ]
            then
                echo $s > /var/run/network_section
                tab_section="$tab_section $s-Devices no"
                INTERFACES="$SEC"

                # check if $FORM_action is one of $INTERFACES
                found=0
                for iface in $INTERFACES
                do
                    if [ "$iface" = "$FORM_action" ]; then
                        found=1
                        break
                    fi
                done
                if [ "$found" -eq 0 ]; then
                    FORM_action=""
                fi
            else
                tab_section=`echo "$tab_section $s-Devices $myname?section=$s"`
            fi
        fi
    done
    
    tab_list=""
    for i in $INTERFACES
    do
        : ${FORM_action:=$i}
        var_name_tmp=`echo $i | sed -e 's/\./_/'`
        var_name=`echo $var_name_tmp | sed -e 's/[.-]/_/'`
        alias_name=""
        net_alias_lookup_name $i alias_name
        if [ "x$alias_name" != "x" ]
        then
            dname=`echo "${alias_name}&nbsp;[$i]"`
        else
            dname=$i
        fi
        eval local rrd_source_time='$FORM_rrd_graphtime_'$rrd_source
        : ${rrd_source_time:=$rrd_default_graphtime}
        eval : \${FORM_rrd_graphtime_$var_name:=$rrd_default_graphtime}
        if [ "$FORM_action" = "$i" ]
        then
            tab_list=`echo "$tab_list $dname no"`
        else
            tab_list=`echo "$tab_list $dname $myname?action=$i&amp;section=$(url_encode "$FORM_section")"`
        fi
    done

    show_tab_header $tab_section
    
    show_tab_header $tab_list
    
    rrd_open_tab_list $FORM_action
    rrd_render_graph network $FORM_action
    rrd_close_tab_list
    show_tab_footer
    
    show_tab_footer
fi

show_html_footer

