#!/bin/sh
#------------------------------------------------------------------------------
# log_boot-update,cgi - show log of events "update" and "boot finished"
#
# Creation:     20007-10-18 lanspezi
# Last Update:  $Id$
#
#------------------------------------------------------------------------------

# get main helper functions
. /srv/www/include/cgi-helper

format_output()
{
    htmlspecialchars | while read line
    do
        dat=`echo $line | cut -d "|" -f 1`
        event=`echo $line | cut -d "|" -f 2`
        version=`echo $line | cut -d "|" -f 3`
        kernel=`echo $line | cut -d "|" -f 4`
        arch=`echo $line | cut -d "|" -f 5`

        echo "<tr>"
        echo "<td>$dat</td>"
        echo "<td>$event</td>"
        echo "<td>$version</td>"
        echo "<td>$kernel</td>"
        echo "<td>$arch</td>"
        echo "</tr>"
    done
}
: ${FORM_action:=view}

# Security
check_rights "logs" "$FORM_action"

# Check if logip.log exist
if [ ! -f /var/lib/persistent/base/boot_upd.log ]
then
    show_html_header "$_MN_err"
    show_error "$_MN_err" "boot_upd.log $_MM_notexist."
    show_html_footer
    exit 1
fi

ATIME=1
tmp="/tmp/$myname.$$"


case $FORM_action in
    reset|$_MN_reset)
        show_html_header "$_MN_resetting /var/lib/persistent/base/boot_upd.log ..." "refresh=$ATIME;url=$myname"
        > /var/lib/persistent/base/boot_upd.log
        show_info "" "$_MN_resetting /var/lib/persistent/base/boot_upd.log"
        show_html_footer
    ;;
    download|$_MN_download)
        http_header download "ctype=text/plain;filename=boot_upd.log"
        cat /var/lib/persistent/base/boot_upd.log
        ;;
    *)
        : ${FORM_sort:=reverse}
        case $FORM_sort in reverse) sort="normal" ;; *) sort="reverse" ;; esac
        # show all rows if the "all rows" button is pressed
        case $FORM_showall in "") ;; *) FORM_lines="$_MN_all" ;; esac
        # sanitize no. of rows: show the default of 50 rows if nothing or nonsens (no integer)  is specified
        case $FORM_lines in all|"$_MN_all") ;; *[^0-9]*|"") FORM_lines=30 ;; esac
        case $FORM_lines in
            all|"$_MN_all"|*[^0-9]*)
                # is "all"
                n="$_MN_all"
                case $FORM_sort in
                    normal)
                        cat /var/lib/persistent/base/boot_upd.log | format_output > /tmp/boot_upd.$$
                    ;;
                    *)
                        cat /var/lib/persistent/base/boot_upd.log | sed '1!G;h;$!d' | format_output > /tmp/boot_upd.$$
                    ;;
                esac
            ;;
            *[0-9]*)
                # is an integer
                n=$FORM_lines
                case $FORM_sort in
                    normal)
                        cat /var/lib/persistent/base/boot_upd.log | do_tail $n | format_output > /tmp/boot_upd.$$
                    ;;
                    *)
                        cat /var/lib/persistent/base/boot_upd.log | do_tail $n | sed '1!G;h;$!d' | format_output > /tmp/boot_upd.$$
                    ;;
                esac
            ;;
        esac
        show_html_header "$_BOOTUPD_maintitle"
        show_tab_header "$_BOOTUPD_title" no
        echo "<h2>$_BOOTUPD_maintitle ($n $_MN_rows)</h2>"
        cat <<-EOF
	<form action="$myname" method="GET">
	    <input class="itext" type="text" size="3" maxlength="4" name="lines" value="$FORM_lines">&nbsp;$_MN_rows&nbsp;&nbsp;
	    <input class="sbutton" type="submit" value="$_MN_show">
	    <input class="sbutton" type="submit" name="showall" value="$_MN_BTN_showallrows">
	    <input class="sbutton" type="submit" name="action" value="$_MN_download">
	    <input class="sbutton" type="submit" name="action" value="$_MN_reset">
	    <input type="hidden" name="sort" value="$FORM_sort">
	</form>
EOF
        echo '<table class="normtable">'
        echo "<tr>"
        echo "<th>$_BOOTUPD_date</th>"
        echo "<th>$_BOOTUPD_event</th>"
        echo "<th>$_BOOTUPD_version</th>"
        echo "<th>$_BOOTUPD_kernel</th>"
        echo "<th>$_BOOTUPD_arch</th>"
        echo "</tr>"
        cat /tmp/boot_upd.$$
        rm /tmp/boot_upd.$$
        echo "</table>"
        show_tab_footer
        show_html_footer
    ;;
esac
