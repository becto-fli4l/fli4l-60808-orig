#!/bin/sh
#------------------------------------------------------------------------------
# status_qos.cgi - Display Status of Network (rrdtool graph)
#
# Creation:     2014-09-02 Lanspezi
# Last Update:  $Id$
#
#------------------------------------------------------------------------------
# get main helper functions
# set_debug=yes

. /srv/www/include/cgi-helper

SEC_ACTION='view'

headtitle="QOS-Status"

# Security
check_rights "status" "$SEC_ACTION"

show_html_header "$headtitle" "${ref}"

#-------------------------------------------------------------------------------
#----------- QOS Graphs ----------------------------------------------------
#-------------------------------------------------------------------------------

if [ -e /srv/www/include/rrd-status_qos.inc ]
then
    . /srv/www/include/rrd-common.inc 
    . /srv/www/include/rrd-status_qos.inc
    . /var/run/qos.conf
    
    # Richtung   Device   Direction  VAR-Bandbreite          VAR_DEFAULT-CLASS
    ###################################################################################
    # Upload     imq1     outbound   QOS_INTERNET_BAND_UP    QOS_INTERNET_DEFAULT_UP
    # Download   imq0     inbound    QOS_INTERNET_BAND_DOWN  QOS_INTERNET_DEFAULT_DOWN
    
    eval ndef=`expr $QOS_CLASS_N + 1`
    
    
    if [ "$FORM_direction" = "" ]
    then
        if [ -f /var/run/rrdtool_qosdirection ]
        then
            eval FORM_direction=`cat /var/run/rrdtool_qosdirection`
            rm /var/run/rrdtool_qosdirection
        fi
    fi
    : ${FORM_direction:=down}
    : ${FORM_action:=$ndef}
    
    tab_list=""
    
    if [ "$FORM_direction" = "down" ]
    then
        dir_list="Download&nbsp;[Max:${QOS_INTERNET_BAND_DOWN}] no Upload&nbsp;[Max:${QOS_INTERNET_BAND_UP}] $myname?direction=up"
        device=imq0
        echo "down" > /var/run/rrdtool_qosdirection
    else
        dir_list="Download&nbsp;[Max:${QOS_INTERNET_BAND_DOWN}] $myname?direction=down Upload&nbsp;[Max:${QOS_INTERNET_BAND_UP}] no"
        device=imq1
        echo "up" > /var/run/rrdtool_qosdirection
    fi

    show_tab_header $dir_list

    tc class list dev $device | grep "htb" | cut -d " " -f3-5 | cut -d ":" -f2,3 | sort -n -r > /tmp/rrdqos.$$ 
    while read class typetmp parenttmp
    do
        parent=""
        if [ "$typetmp" = "parent" ]
        then
            parent=`echo "$parenttmp" | cut -d ":" -f2`
            tablabel=`echo "CLASS:$class - Sub of CLASS:$parent"`
        else
            tablabel=`echo "CLASS:$class - ROOTCLASS"`
        fi
        eval labeltmp='$QOS_CLASS_'$class'_LABEL'
        if [ "x$labeltmp" != "x" ]
        then
            tablabel=`echo "$tablabel [$labeltmp]" | sed -e 's/ /\&nbsp;/g'`
        else
            tablabel=`echo "$tablabel" | sed -e 's/ /\&nbsp;/g'`
        fi
        if [ $class -eq $FORM_action ]
        then
            tab_list=`echo "$tab_list $tablabel no"`
        else
            tab_list=`echo "$tab_list $tablabel $myname?direction=${FORM_direction}&action=${class}"`
        fi
        eval local rrd_source_time='$FORM_rrd_graphtime_'$rrd_source
        : ${rrd_source_time:=$rrd_default_graphtime}
        eval : \${FORM_rrd_graphtime_$class:=$rrd_default_graphtime}
        
    done < /tmp/rrdqos.$$
     rm /tmp/rrdqos.$$
     
       
    if [ "x$tab_list" = "x" ]
    then
        show_info "Info" "Keine QOS_CLASS mit DIRECTION='$FORM_direction' gefunden"
    else
        show_tab_header $tab_list
    
        rrd_open_tab_list $FORM_action
        rrd_render_graph qos$FORM_direction $FORM_action
        rrd_close_tab_list
        show_tab_footer
    fi
    
    show_tab_footer
fi

show_html_footer

