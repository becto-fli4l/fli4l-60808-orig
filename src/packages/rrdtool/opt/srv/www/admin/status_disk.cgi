#!/bin/sh
#------------------------------------------------------------------------------
# status_system.cgi - Display Status of System (rrdtool graph)
#
# Creation:     2009.09.01 Lanspezi
# Last Update:  $Id$
#
#------------------------------------------------------------------------------
# get main helper functions
# set_debug=yes

. /srv/www/include/cgi-helper

# Security
check_rights "status" "view"

show_html_header "Festplattenstatus" "${ref}"

#-------------------------------------------------------------------------------
#----------- Disk Graphs -------------------------------------------------------
#-------------------------------------------------------------------------------

if [ -e /srv/www/include/rrd-status_disk.inc ]
then
    . /srv/www/include/rrd-common.inc 
    . /srv/www/include/rrd-status_disk.inc 

    : ${FORM_rrd_graphtime_partitionboot:=$rrd_default_graphtime}
    : ${FORM_rrd_graphtime_partitiondata:=$rrd_default_graphtime}
    : ${FORM_rrd_graphtime_partitionroot:=$rrd_default_graphtime}
    : ${FORM_rrd_graphtime_partitionopt:=$rrd_default_graphtime}

    _partitionboot="Partition:&nbsp;/boot"
    _partitiondata="Partition:&nbsp;/data"
    _partitionroot="Partition:&nbsp;/"
    _partitionopt="Partition:&nbsp;/opt"

    : ${FORM_action:="partitionroot"}

    if [ "$FORM_action" = "partitionroot" ]
    then
        tab_list="$_partitionroot no "
    else
        tab_list="$_partitionroot $myname?action=partitionroot "
    fi
    if  [ "$FORM_action" = "partitionboot" ]
    then
        tab_list=`echo "$tab_list $_partitionboot no"`
    else
        tab_list=`echo "$tab_list $_partitionboot $myname?action=partitionboot"`
    fi

    if `df |grep -q "/data"`
    then
        if  [ "$FORM_action" = "partitiondata" ]
        then
            tab_list=`echo "$tab_list $_partitiondata no"`
        else
            tab_list=`echo "$tab_list $_partitiondata $myname?action=partitiondata"`
        fi
    fi
    if `df | grep -q "/opt"`
    then
        if  [ "$FORM_action" = "partitionopt" ]
        then
            tab_list=`echo "$tab_list $_partitionopt no"`
        else
            tab_list=`echo "$tab_list $_partitionopt $myname?action=partitionopt"`
        fi
    fi

    show_tab_header $tab_list

    rrd_open_tab_list $FORM_action
    rrd_render_graph $FORM_action
    rrd_close_tab_list
    show_tab_footer
fi

show_html_footer

