#!/bin/sh
#------------------------------------------------------------------------------
# status.cgi - Display Status of ISDN & DSL
#
# Creation:     17.01.2001  tg
# Last Update:  $Id$
#
#------------------------------------------------------------------------------
# get main helper functions
. /srv/www/include/cgi-helper

#---------------------------------------------------------------------------------------------
#------------------------- Actions -----------------------------------------------------------
#---------------------------------------------------------------------------------------------

case $FORM_action in
    "$_STAT_reboot"|"$_STAT_poweroff")
        SEC_ACTION='boot'
        ref="refresh=1;url=/admin/index.cgi?action=logout"
    ;;
    *)
        FORM_action='view'
        SEC_ACTION='view'
    ;;
esac

# Security
check_rights "control" "$SEC_ACTION"
show_html_header "" "${ref}"

case "$FORM_action" in
    "$_STAT_reboot")
        show_info "" "$_STAT_reboot_req $REMOTE_ADDR ($REMOTE_USER) -- $_STAT_rebooting ... "
        show_html_footer
	cgi_log "$_STAT_reboot_req $REMOTE_ADDR ($REMOTE_USER) -- $_STAT_rebooting ... "
        ( sleep 5 ; reboot )&
    ;;
    "$_STAT_poweroff")
        show_info "" "$_STAT_poweroff_req $REMOTE_ADDR ($REMOTE_USER) -- $_STAT_poweroffing ... "
        show_html_footer
        cgi_log "$_STAT_poweroff_req $REMOTE_ADDR ($REMOTE_USER) -- $_STAT_poweroffing ... "
        ( sleep 5; poweroff )&
    ;;
    view)
        #---------------------------------------------------------------------------------------------
        #------------------------- Actions -----------------------------------------------------------
        #---------------------------------------------------------------------------------------------

        show_tab_header "$_STAT_actions" no

        # Poweroff/Reboot
        echo '<form action="'$myname'" method="GET">'
        echo '<input type="submit" value="'$_STAT_reboot'" name="action" class="actions">'
        echo '<input type="submit" value="'$_STAT_poweroff'" name="action" class="actions">'
        echo '</form>'
        show_tab_footer
        echo '<br>'
        show_html_footer
    ;;
esac
