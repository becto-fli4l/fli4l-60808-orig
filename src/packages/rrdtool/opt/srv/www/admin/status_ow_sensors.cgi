#!/bin/sh
#------------------------------------------------------------------------------
# status_ow_sensors.cgi - Display Status of OW-Sensors (rrdtool graph)
#
# Creation:     2015-02-20 Franke
# Last Update:  $Id$
#
#------------------------------------------------------------------------------
# get main helper functions
# set_debug=yes

. /srv/www/include/cgi-helper

#SEC_ACTION='view'

#headtitle="OWFS_Sensor_Grafiken"

# Security
check_rights "status" "view"

#show_html_header "$headtitle" "${ref}"

show_html_header "OWFS_Sensor_Grafiken"

#-------------------------------------------------------------------------------
#----------- OW_SENSOR Graphs ----------------------------------------------------
#-------------------------------------------------------------------------------

if [ -e /srv/www/include/rrd-status_owfs.inc ]
then
    . /srv/www/include/rrd-common.inc 
    . /srv/www/include/rrd-status_owfs.inc 
    
    grp_list=""
   	tab_list=""
    if [ "$FORM_direction" = "" ]
    then
        if [ -f /var/run/ow_owfs_section ]
        then
            eval FORM_direction=`cat /var/run/ow_owfs_section`
            rm /var/run/ow_owfs_section
        fi
    fi
    : ${FORM_direction:=ALLES}
    if [ "$FORM_direction" = "ALLES" ]
    then
        grp_list=`echo "ALLES no GRUPPE $myname?direction=GRUPPE"`
        echo "GRUPPE" > /var/run/ow_owfs_section
    else
        grp_list=`echo "ALLES $myname?direction=ALLES GRUPPE no"`
        echo "ALLES" > /var/run/ow_owfs_section
    fi

    show_tab_header $grp_list

    if [ "$FORM_direction" = "ALLES" ]
    then 
    for int1 in `ls -d ${rrd_dbpath}/onewire-* | grep -o '............$'`
    do
        : ${FORM_action:=$int1}
        eval local rrd_source_time='$FORM_rrd_graphtime_'$rrd_source
        : ${rrd_source_time:=$rrd_default_graphtime}
        eval : \${FORM_rrd_graphtime_$int1:=$rrd_default_graphtime}
        file=/var/run/ow_$int1
        if [ -f $file ]
        then 
           int_name=`cat $file`
        else
           int_name="Sensor:"$int1
        fi
        if [ "$FORM_action" = "$int1" ]
        then
            tab_list=`echo "$tab_list $int_name no"`
        else
            tab_list=`echo "$tab_list $int_name $myname?action=$int1&amp;direction=ALLES"`
        fi
    done
    echo "ALLES" > /var/run/ow_owfs_section
    show_tab_header $tab_list
    
    rrd_open_tab_list $FORM_action
    rrd_render_graph temperature $FORM_action
    fi

    if [ "$FORM_direction" = "GRUPPE" ]
    then 
        for int2 in `ls -1 /var/run/owfs_rrdtool_group`
        do
            for d_sensor in `ls -1 /var/run/owfs_rrdtool_group/$int2/*temperature.rrd | cut -d "/" -f6 | head -1`
            do 
                    : ${FORM_action:=$int2}
                    eval local rrd_source_time='$FORM_rrd_graphtime_'$rrd_source
                    : ${rrd_source_time:=$rrd_default_graphtime}
                    eval : \${FORM_rrd_graphtime_$int2:=$rrd_default_graphtime}
                    grp_name=`echo "$int2" | cut -c 4-`
                    if [ "$FORM_action" = "$int2" ]
                    then
                        tab_list_grp=`echo "$tab_list_grp $grp_name no"`
                    else
                        tab_list_grp=`echo "$tab_list_grp $grp_name $myname?action=$int2&amp;direction=GRUPPE"`
                    fi
            done
        done
    echo "GRUPPE" > /var/run/ow_owfs_section
    show_tab_header $tab_list_grp
    
    rrd_open_tab_list $FORM_action
    rrd_render_graph temperature_grp $FORM_action
    fi

    rrd_close_tab_list
    show_tab_footer
    show_tab_footer
fi

show_html_footer

