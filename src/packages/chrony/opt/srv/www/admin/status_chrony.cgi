#!/bin/sh
. /srv/www/include/cgi-helper
check_rights "chrony" "view"
show_html_header "$_CHRONY_title"
: ${FORM_action:=activity}
tab_list=
for act in activity sources sourcestats tracking
do
    if [ "$FORM_action" == "$act" ]
    then
        tab_list=`echo "$tab_list $act no"`
    else
        tab_list=`echo "$tab_list $act $myname?action=$act"`
    fi
done
show_tab_header $tab_list
echo '<pre>'
/usr/bin/chronyc $FORM_action -v | grep -v "200 OK"
echo '</pre>'
show_tab_footer
show_html_footer
