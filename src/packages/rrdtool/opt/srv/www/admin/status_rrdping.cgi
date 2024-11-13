#!/bin/sh
#------------------------------------------------------------------------------
# status_rrdping.cgi - Display Status Hosts-Ping (rrdtool graph)
#
# Creation:     2010.08.08 Lanspezi
# Last Update:  $Id$
#
#------------------------------------------------------------------------------
# get main helper functions
# set_debug=yes

. /srv/www/include/cgi-helper

SEC_ACTION='view'

headtitle="Host-Ping-Status"

# Security
check_rights "status" "$SEC_ACTION"

show_html_header "$headtitle" "${ref}"

#-------------------------------------------------------------------------------
#----------- Ping Graphs -------------------------------------------------------
#-------------------------------------------------------------------------------

if [ -e /srv/www/include/rrd-status_ping.inc ]
then
    . /srv/www/include/rrd-common.inc 
    . /srv/www/include/rrd-status_ping.inc 

    . /var/run/rrdtool_ping.conf
    
    tab_list=""
    grp_list=""
    if [ "$FORM_grpnr" = "" ]
    then
        if [ -f /var/run/rrdtool_pinggroup ]
        then
            eval FORM_grpnr=`cat /var/run/rrdtool_pinggroup`
            rm /var/run/rrdtool_pinggroup
        fi
    fi
    : ${FORM_grpnr:=1}
    
    [ 0$RRDTOOL_PINGGROUP_N -eq 0 ] || for idn in `seq 1 $RRDTOOL_PINGGROUP_N`
    do
        eval grpnametmp='$RRDTOOL_PINGGROUP_'$idn'_LABEL'
        grpname=`echo "$grpnametmp" | sed -e 's/ /\&nbsp;/g'`
        if [ "$FORM_grpnr" = "$idn" ]
        then
            grp_list=`echo "$grp_list $grpname no"`
            echo $idn > /var/run/rrdtool_pinggroup
        else
            grp_list=`echo "$grp_list $grpname $myname?grpnr=$idn&amp;"`
        fi
    done
    
    [ 0$RRDTOOL_PING_N -eq 0 ] || for idx in `seq 1 $RRDTOOL_PING_N`
    do
        eval grphost='$RRDTOOL_PING_'$idx'_GRPNR'
        if [ 0$RRDTOOL_PINGGROUP_N -eq 0 ]
        then
            grphost=1
        fi
        if [ "$grphost" = "$FORM_grpnr" ]
        then
            eval hname='$RRDTOOL_PING_'$idx
            : ${FORM_action:=$hname}
        
            hnamemod=`echo $hname | sed -e 's/[.-]/_/g'`
            eval local rrd_source_time='$FORM_rrd_graphtime_'$rrd_source
            : ${rrd_source_time:=$rrd_default_graphtime}
            eval : \${FORM_rrd_graphtime_$hnamemod:=$rrd_default_graphtime}
            eval label='$RRDTOOL_PING_'$idx'_LABEL'
            : ${label:=""}
            if [ "x${label}" != "x" ]
            then
                tablabel=`echo "${label} [${hname}]" | sed -e 's/ /\&nbsp;/g'`
            else
                tablabel=`echo "${hname}"`
            fi
            if [ "$FORM_action" = "$hname" ]
            then
                tab_list=`echo "$tab_list $tablabel no"`
            else
                tab_list=`echo "$tab_list $tablabel $myname?action=$hname&amp;grpnr=$FORM_grpnr&amp;"`
            fi
        fi
    done
    
    show_tab_header $grp_list
    show_tab_header $tab_list
    
    rrd_open_tab_list $FORM_action
    rrd_render_graph rrdping $FORM_action
    rrd_close_tab_list
    show_tab_footer
    show_tab_footer
fi

show_html_footer

