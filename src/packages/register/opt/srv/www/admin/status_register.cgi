#!/bin/sh
#------------------------------------------------------------------------------
# opt/srv/www/admin/status_register.cgi                            __FLI4LVER__
#
# Last Update:  $Id$
#------------------------------------------------------------------------------

. /srv/www/include/cgi-helper

# Security
check_rights "register" "view"

# keep synchronised with opt/usr/local/bin/registerc_once
DATA=/var/run/registerc.last
# maximum number of parts in a multi-part value
MAX_PART_COUNT=2

show_html_header "REGISTER"

if [ -s "$DATA" ]
then
    show_tab_header "$_REGISTER_TITLE" no

    echo "<table class='normtable'>"
    echo "<tr><th>$_REGISTER_FIELD</th><th>$_REGISTER_DESCRIPTION</th><th colspan='${MAX_PART_COUNT}'>$_REGISTER_VALUE</th></tr>"
    while read -r line
    do
        line=$(echo "$line" | htmlspecialchars)
        set -- $line
        field=$1
        eval desc=\$_REGISTER_DESCRIPTION_${field}
        shift
        numvalues=$#
        lastspan=$((MAX_PART_COUNT - numvalues + 1))

        echo "<tr><td class='register_field'><center>${field}</center></td><td class='register_desc'>${desc}</td>"
        i=1
        for value
        do
            if [ $i -eq $numvalues -a $lastspan -gt 1 ]
            then
                echo "<td class='register_value' colspan='${lastspan}'>${value}</td>"
            else
                echo "<td class='register_value'>${value}</td>"
            fi
            i=$((i+1))
        done
        echo "</tr>"
    done < "$DATA"
    echo "</table>"

    show_tab_footer
else
    show_error "" "$_REGISTER_NO_DATA"
fi

echo "<p><a href='https://register.fli4l.de/'>${_REGISTER_LINK_TO_WEB_SITE}</a></p>"
show_html_footer
