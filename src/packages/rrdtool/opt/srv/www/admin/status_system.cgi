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

show_html_header "Systemstatus" "${ref}"

#-------------------------------------------------------------------------------
#----------- Sytem & Load Graphs -----------------------------------------------
#-------------------------------------------------------------------------------

if [ -e /srv/www/include/rrd-status_system.inc ]
then
    . /srv/www/include/rrd-common.inc 
    . /srv/www/include/rrd-status_system.inc 

    cpu_anz=`grep -cE '^[Pp]rocessor[[:space:]]+:' /proc/cpuinfo`
    cpu_anzr=`expr $cpu_anz - 1`
    for n in `seq 0 $cpu_anzr`
    do
        eval : \${FORM_rrd_graphtime_cpu$n:=$rrd_default_graphtime}
        eval : \${_cpu$n:="cpu_$n"}
    done

    : ${FORM_rrd_graphtime_load:=$rrd_default_graphtime}
    : ${FORM_rrd_graphtime_cpuall:=$rrd_default_graphtime}
    : ${FORM_rrd_graphtime_uptime:=$rrd_default_graphtime}
    : ${FORM_rrd_graphtime_memory:=$rrd_default_graphtime}
    : ${FORM_rrd_graphtime_processes:=$rrd_default_graphtime}
    : ${FORM_rrd_graphtime_entropy:=$rrd_default_graphtime}

    _load="load"
    _cpuall="cpu_ALL"
    _uptime="uptime"
    _memory="memory"
    _processes="processes"
    _entropy="entropy"

    : ${FORM_action:=$_load}
    # sanitize $FORM_action so it can be safely used
    FORM_action=$(echo "$FORM_action" | sed 's/[^a-zA-Z0-9_]//g')

    tab_list=`echo "$_load $myname?action=load"`

    for n in `seq 0 $cpu_anzr`
    do
        tab_list=`echo "$tab_list cpu_$n $myname?action=cpu$n "`
    done
    if [ $cpu_anz -gt 1 ]
    then
        tab_list=`echo "$tab_list $_cpuall $myname?action=cpuall "`
    fi
    show_tab_header $tab_list \
                    $_uptime $myname?action=uptime \
                    $_memory $myname?action=memory \
                    $_processes $myname?action=processes \
                    $_entropy $myname?action=entropy \

    rrd_open_tab_list $FORM_action
    case $FORM_action in
        cpuall)
            rrd_render_graph $FORM_action
            ;;
        cpu*)
            rrd_render_graph cpu $FORM_action
            ;;
        *)
            rrd_render_graph $FORM_action
            ;;
    esac
    rrd_close_tab_list
    show_tab_footer
fi

show_html_footer

