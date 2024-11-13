#!/bin/sh
#------------------------------------------------------------------------------
# status_rrdapcups.cgi - Display APCUPS graph's
#
# Creation:     2012-07-017 Lanspezi
# Last Update:  $Id$
#
#------------------------------------------------------------------------------
# get main helper functions
# set_debug=yes

. /srv/www/include/cgi-helper

SEC_ACTION='view'

headtitle="Status APCUPS"

# Security
check_rights "status" "$SEC_ACTION"

show_html_header "$headtitle" "${ref}"

#-------------------------------------------------------------------------------
#----------- Apcups Graphs -----------------------------------------------------
#-------------------------------------------------------------------------------

if [ -e /srv/www/include/rrd-status_apcups.inc ]
then
    . /srv/www/include/rrd-common.inc 
    . /srv/www/include/rrd-status_apcups.inc 

    : ${FORM_rrd_graphtime_apcups_charge:=$rrd_default_graphtime}
    : ${FORM_rrd_graphtime_apcups_frequency:=$rrd_default_graphtime}
    : ${FORM_rrd_graphtime_apcups_load:=$rrd_default_graphtime}
    : ${FORM_rrd_graphtime_apcups_temperature:=$rrd_default_graphtime}
    : ${FORM_rrd_graphtime_apcups_timeleft:=$rrd_default_graphtime}
    : ${FORM_rrd_graphtime_apcups_batvolt:=$rrd_default_graphtime}
    : ${FORM_rrd_graphtime_apcups_pwrvolt:=$rrd_default_graphtime}
    
    _apcups_charge="apcups_charge"
    _apcups_frequency="apcups_frequency"
    _apcups_load="apcups_load"
    _apcups_temperature="apcups_temperature"
    _apcups_timeleft="apcups_timeleft"
    _apcups_batvolt="apcups_batvolt"
    _apcups_pwrvolt="apcups_pwrvolt"
    
    : ${FORM_action:=$_apcups_load}
    
    tab_list=""
    
    for tab in charge frequency load temperature timeleft batvolt pwrvolt
    do
        eval tabn=\${_apcups_$tab}
        case $tab in
            charge)
                eval label=$_RRD3_RT_CHARGE
                ;;
            frequency)
                eval label=$_RRD3_RT_FREQUENCY
                ;;
            load)
                eval label=$_RRD3_RT_LOAD
                ;;
            temperature)
                eval label=$_RRD3_RT_TEMPERATUE
                ;;
            timeleft)
                eval label=$_RRD3_RT_TIMELEFT
                ;;
            batvolt)
                eval label=$_RRD3_RT_BATVOLT
                ;;
            pwrvolt)
                eval label=$_RRD3_RT_PWRVOLT
                ;;
        esac
        if [ "$FORM_action" = "$tabn" ]
        then
            tab_list=`echo "$tab_list $label no"`
        else
            tab_list=`echo "$tab_list $label $myname?action=$tabn&amp;"`
        fi
    done

    show_tab_header $tab_list

    rrd_open_tab_list $FORM_action
    rrd_render_graph $FORM_action
    rrd_close_tab_list
    show_tab_footer
fi

show_html_footer

