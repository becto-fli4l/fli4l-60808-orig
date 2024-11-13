#!/bin/sh
#------------------------------------------------------------------------------
# status_rrdpperping.cgi - Display Status Peer-Ping (rrdtool graph)
#
# Creation:     2012.07.18 Lanspezi
# Last Update:  $Id$
#
#------------------------------------------------------------------------------
# get main helper functions
# set_debug=yes

. /srv/www/include/cgi-helper

SEC_ACTION='view'

headtitle="Peer-Ping-Status"

# Security
check_rights "status" "$SEC_ACTION"

show_html_header "$headtitle" "${ref}"

#-------------------------------------------------------------------------------
#----------- Ping Graphs -------------------------------------------------------
#-------------------------------------------------------------------------------

if [ -e /srv/www/include/rrd-status_peerping.inc ]
then
    . /srv/www/include/rrd-common.inc 
    . /srv/www/include/rrd-status_peerping.inc 

    . /var/run/rrdtool_peerping.conf
    
    tab_list=""
    [ 0$RRDTOOL_PEERPING_N -eq 0 ] || for idx in `seq 1 $RRDTOOL_PEERPING_N`
    do
        eval hname='$RRDTOOL_PEERPING_'$idx
        : ${FORM_action:=$hname}
        
        hnamemod=`echo $hname | sed -e 's/[.-]/_/g'`
        eval local rrd_source_time='$FORM_rrd_graphtime_'$rrd_source
        : ${rrd_source_time:=$rrd_default_graphtime}
        eval : \${FORM_rrd_graphtime_$hnamemod:=$rrd_default_graphtime}
        eval label='$RRDTOOL_PEERPING_'$idx'_LABEL'
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
            tab_list=`echo "$tab_list $tablabel $myname?action=$hname&amp;"`
        fi
    done
    
    show_tab_header $tab_list
    
    rrd_open_tab_list $FORM_action
    rrd_render_graph rrdpeerping $FORM_action
    rrd_close_tab_list
    show_tab_footer
fi

show_html_footer            

