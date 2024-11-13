#!/bin/sh
#------------------------------------------------------------------------------
# status_irq.cgi - Display Status of irq's (rrdtool graph)
#
# Creation:     2010.08.08 Lanspezi
# Last Update:  $Id$
#
#------------------------------------------------------------------------------
# get main helper functions
# set_debug=yes

. /srv/www/include/cgi-helper

SEC_ACTION='view'

headtitle="Interrupts"

# Security
check_rights "status" "$SEC_ACTION"

show_html_header "$headtitle" "${ref}"

#-------------------------------------------------------------------------------
#-----------Interrupt Graphs ---------------------------------------------------
#-------------------------------------------------------------------------------

if [ -e /srv/www/include/rrd-status_irq.inc ]
then
    . /srv/www/include/rrd-common.inc 
    . /srv/www/include/rrd-status_irq.inc 

    tab_list=""
    for int in `sed '1d;s/:.*//;s/^ *//' </proc/interrupts`
    do
        if [ -f ${rrd_dbpath}/irq/irq-$int.rrd ]
        then
            : ${FORM_action:=$int}
            eval local rrd_source_time='$FORM_rrd_graphtime_'$rrd_source
            : ${rrd_source_time:=$rrd_default_graphtime}
            eval : \${FORM_rrd_graphtime_$int:=$rrd_default_graphtime}
            desc=`sed -n "/^ *$int:/s/^.*  [0-9]*//p" </proc/interrupts | sed 's/ interrupts$//;s/ /\&nbsp;/g'`
            if [ "x$desc" = "x" ]
            then
                desc="-"
            fi
            if [ "$FORM_action" = "$int" ]
            then
                tab_list=`echo "$tab_list $int&nbsp;[&nbsp;${desc}&nbsp;] no"`
            else
                tab_list=`echo "$tab_list $int&nbsp;[&nbsp;${desc}&nbsp;] $myname?action=$int&amp;"`
            fi
        fi
    done

    show_tab_header $tab_list

    rrd_open_tab_list $FORM_action
    rrd_render_graph irq $FORM_action
    rrd_close_tab_list
    show_tab_footer
fi

show_html_footer
