#!/bin/sh
# $Id$
# Do not edit! This file is automaticly generated by rrd-cgi.xsl

. /srv/www/include/cgi-helper
# set_debug=yes

# Security
check_rights "hwsupp" "view"

# get some internal variables
. /var/run/hwsupp.conf

show_html_header "PC Engines WRAP"

if [ -e /srv/www/include/rrd-common.inc ]
then
    . /srv/www/include/rrd-common.inc
    if [ -e /srv/www/include/rrd-pcengines-wrap.inc ]
    then
     . /srv/www/include/rrd-pcengines-wrap.inc
    fi
fi
if [ -e /srv/www/include/hwmon-pcengines-wrap.inc ]
then
    . /srv/www/include/hwmon-pcengines-wrap.inc
fi
if [ -e /srv/www/include/extra-dmidecode.inc ]
then
    . /srv/www/include/extra-dmidecode.inc
fi
if [ -e /srv/www/include/extra-pcengines-wrap.inc ]
then
    . /srv/www/include/extra-pcengines-wrap.inc
fi

: ${FORM_action:=pcengines_wrap_temp}
if [ -e /srv/www/include/rrd-common.inc ]
then
    : ${FORM_rrd_graphtime_pcengines_wrap_temp:=$rrd_default_graphtime}

    eval local rrd_source_time='$FORM_rrd_graphtime_'$rrd_source
    : ${rrd_source_time:=$rrd_default_graphtime}
fi
action_list=""
if [ -e /srv/www/include/rrd-common.inc ]
then
    action_list="$action_list temp"
fi
action_list="$action_list hwmon"
action_list="$action_list bios"
action_list="$action_list wrap"

tab_list=""
for i in ${action_list}
do
    case $i in
        temp)
            label=$(translate_label "${_HWSUPP_TEMP}")
        ;;
        hwmon)
            label=$(translate_label "${_HWSUPP_HWMON}")
        ;;
        bios)
            label=$(translate_label "${_HWSUPP_DMI}")
        ;;
        wrap)
            label=$(translate_label "wrap")
        ;;
    esac
    if [ "$FORM_action" = "pcengines_wrap_$i" ]
    then
        tab_list=`echo "$tab_list $label no"`
    else
        tab_list=`echo "$tab_list $label $myname?action=pcengines_wrap_$i"`
    fi
done

show_tab_header $tab_list
case $FORM_action in
    pcengines_wrap_hwmon)
        hwmon_pcengines_wrap
    ;;
    pcengines_wrap_bios)
        extra_dmidecode
    ;;
    pcengines_wrap_wrap)
        extra_pcengines_wrap
    ;;
    *)
        rrd_open_tab_list $FORM_action
        rrd_render_graph $FORM_action
        rrd_close_tab_list
    ;;
esac

show_tab_footer

show_html_footer

# _oOo_
