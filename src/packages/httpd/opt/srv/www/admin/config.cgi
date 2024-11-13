#!/bin/sh
#------------------------------------------------------------------------------
# config.cgi - Change dialmode and default-circuit, etc.
#
# Creation:     03.02.2001  tg
# Last Update:  $Id$
#
#------------------------------------------------------------------------------

# get main helper functions
. /srv/www/include/cgi-helper
. /srv/www/include/config-helper.inc

myname=`basename $0`

show_html_header "DSL-Konfiguration"

case $FORM_action in
    "")
        show_tab_header "pppoe-config" no
        read_var PPPOE_USER
        echo "<form action=\"$myname\" method=\"POST\">"
        echo "User: <input type=\"text\" value=\"$PPPOE_USER\" name=\"user\"><br>"
        echo "Pass: <input type=\"password\" value=\"****\" name=\"pass\"><br>"
        echo "<input type=\"submit\" value=\"ändern\" name=\"action\">"
        echo "</form>"
        show_tab_footer
    ;;
    ändern)
        PPPOE_USER="$FORM_user"
        write_var PPPOE_USER
        if [ "$FORM_pass" != "****" ]
        then
            PPPOE_PASS="$FORM_pass"
            write_var PPPOE_PASS
        fi
        show_info "info" "Username / Passwort geändert"
    ;;
esac

show_html_footer
