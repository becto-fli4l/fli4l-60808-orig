#!/bin/sh
#------------------------------------------------------------------------------
# opt/files/srv/www/admin/apcupds.cgi                      __FLI4LVER__
#
# Creation:     06.02.2013 cspiess
# Last Update:  $Id$
#------------------------------------------------------------------------------


. /srv/www/include/cgi-helper
#set_debug=yes

# Security
check_rights "apcupsd" "view"

#globals
: ${FORM_action:=apcupsd_info}

# functions 
apcupsd_info ()
{
    if grep -q "^NETSERVER on" /etc/apcupsd/apcupsd.conf
    then
        /sbin/apcaccess | grep -v "END APC" | sed -e "s/\(\w*\) *: *\(.*\) */APCSTATUS_\1=\"\2\"/g" > /tmp/apcupsd.dat
        . /tmp/apcupsd.dat

        linev=`echo "$APCSTATUS_LINEV" | sed -e "s/\([0-9\.]*\).*/\1/g"`
        outputv=`echo "$APCSTATUS_OUTPUTV" | sed -e "s/\([0-9\.]*\).*/\1/g"`
        linefreq=`echo "$APCSTATUS_LINEFREQ" | sed -e "s/\([0-9\.]*\).*/\1/g"`
        battv=`echo "$APCSTATUS_BATTV" | sed -e "s/\([0-9\.]*\).*/\1/g"`
        bcharge=`echo "$APCSTATUS_BCHARGE" | sed -e "s/\([0-9\.]*\).*/\1/g"`
        loadpct=`echo "$APCSTATUS_LOADPCT" | sed -e "s/\([0-9\.]*\).*/\1/g"`
        timeleft=`echo "$APCSTATUS_TIMELEFT" | sed -e "s/\([0-9]*\).*/\1/g"`
    
        show_tab_header "$_APCUPSD_INFO_STATUS" no
        cat <<EOF
    <table class='normtable'>
        <tr><th>$_APCUPSD_INFO_STATUS</th><td>$APCSTATUS_STATUS</td></tr>
        <tr><th>$_APCUPSD_INFO_TIMELEFT</th><td>$timeleft $_APCUPSD_INFO_MINUTES</td></tr>
    </table>
EOF
        show_tab_footer
        show_tab_header "$_APCUPSD_INFO_POWER" no
        cat <<EOF
    <table class='normtable'>
        <tr><th>$_APCUPSD_INFO_LINEV</th><td>$linev V</td></tr>
        <tr><th>$_APCUPSD_INFO_OUTPUTV</th><td>$outputv V</td></tr>
        <tr><th>$_APCUPSD_INFO_LINEFREQ</th><td>$linefreq Hz</td></tr>
    </table>
EOF
        show_tab_footer
        show_tab_header "$_APCUPSD_INFO_LOAD" no
        cat <<EOF
    <table class='normtable'>
        <tr><th>$_APCUPSD_INFO_LOADPCT</th><td>$loadpct %</td></tr>
    </table>
EOF
        show_tab_footer
        show_tab_header "$_APCUPSD_INFO_BATTERY" no
        cat <<EOF
    <table class='normtable'>
        <tr><th>$_APCUPSD_INFO_BATTV</th><td>$battv V</td></tr>
        <tr><th>$_APCUPSD_INFO_BCHARGE</th><td>$bcharge %</td></tr>
    </table>
EOF
        show_tab_footer
        show_tab_header "$_APCUPSD_INFO_INFO" no
        cat <<EOF
    <table class='normtable'>
        <tr><th>$_APCUPSD_INFO_MODEL</th><td>$APCSTATUS_MODEL</td></tr>
        <tr><th>$_APCUPSD_INFO_SERIALNO</th><td>$APCSTATUS_SERIALNO</td></tr>
    </table>
EOF
        show_tab_footer
    else
        show_info "Info not aviable" "please enable NETSERVER in your configuration"
    fi
}

show_html_header "APC UPS Daemon"

tab_list=""

tabs=`echo "info cgi"`

for i in $tabs
do
    case $i in
        info)
            eval label=$_APCUPSD_INFO
            ;;
        cgi)
            eval label="$_APCUPSD_CGIBIN"
            ;;
    esac
    if [ "$FORM_action" = "apcupsd_$i" ]
    then
        tab_list=`echo "$tab_list $label no"`
    else
        tab_list=`echo "$tab_list $label $myname?action=apcupsd_$i"`
    fi
done
show_tab_header $tab_list

case $FORM_action in
    apcupsd_info)
        apcupsd_info
        ;;
    apcupsd_cgi)
        echo "<a href=$myname?action=apcupsd_cgi><br />&nbsp;Zur&uuml;ck zur Multimonitor-Auswahl</a><br />"
        echo '<iframe src=/cgi-bin/opt_apcupsd_iframe.cgi width="900" height="750" frameborder="0">'
        echo "</iframe>"
         ;;
esac
show_tab_footer

show_html_footer
