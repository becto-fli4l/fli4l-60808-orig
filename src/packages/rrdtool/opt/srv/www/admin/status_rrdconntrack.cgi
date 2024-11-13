#!/bin/sh
#------------------------------------------------------------------------------
# status_rrdconntrack.cgi - Display conntrack (rrdtool graph)
#
# Creation:     2009.09.01 Lanspezi
# Last Update:  $Id$
#
#------------------------------------------------------------------------------
# get main helper functions
# set_debug=yes

. /srv/www/include/cgi-helper

SEC_ACTION='view'

headtitle="$_MP_contrckrrd"

# Security
check_rights "status" "$SEC_ACTION"

show_html_header "$headtitle" "${ref}"

#-------------------------------------------------------------------------------
#----------- Sytem & Load Graphs -----------------------------------------------
#-------------------------------------------------------------------------------

if [ -e /srv/www/include/rrd-status_conntrack.inc ]
then
    . /srv/www/include/rrd-common.inc 
    . /srv/www/include/rrd-status_conntrack.inc 

    : ${FORM_rrd_graphtime_conntrack:=$rrd_default_graphtime}
    
    _conntrack="conntrack"
    
    : ${FORM_action:=$_conntrack}

    show_tab_header "$_MP_contrck (List)" conntrack.cgi "$_MP_contrckrrd" no

    rrd_open_tab_list $FORM_action
    rrd_render_graph $FORM_action
    rrd_close_tab_list
    show_tab_footer
fi

show_html_footer

