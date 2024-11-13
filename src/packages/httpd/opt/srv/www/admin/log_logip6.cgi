#!/bin/sh
#------------------------------------------------------------------------------
# logip6.cgi - show log of wan-ip
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
        typ=`echo $line | cut -d "|" -f 2`
        ip=`echo $line | cut -d "|" -f 3`
        interface=`echo $line | cut -d "|" -f 4`
        name=`echo $line | cut -d "|" -f 5`
        is_default_route=`echo $line | cut -d "|" -f 6`

        if [ "x$interface" = "x" ]
        then
            interface="-"
        fi
        if [ "x$is_default_route" = "x" ]
        then
            is_default_route="-"
        fi
        echo "<tr>"
        echo "<td>$dat</td>"
        echo "<td>$typ</td>"
        echo "<td>$ip</td>"
        echo "<td>$interface</td>"
        echo "<td>$name</td>"
        echo "<td>$is_default_route</td>"
        echo "</tr>"
    done
}
: ${FORM_action:=view}

# Security
check_rights "logs" "$FORM_action"

logip_ok=true
if [ -f /var/run/logip.conf ]
then
    . /var/run/logip.conf
else
    logip_ok=false
fi

# Check if logip6.log exist
if [ ! -f ${LOGIP_LOGDIR}/logip6.log -o "$logip_ok" = "false" ]
then
    show_html_header "$_MN_err"
    show_error "$_MN_err" "logip6.log $_MM_notexist."
    show_html_footer
    exit 1
fi

ATIME=1
tmp="/tmp/$myname.$$"


case $FORM_action in
    reset|$_MN_reset)
        show_html_header "$_MN_resetting ${LOGIP_LOGDIR}/logip6.log ..." "refresh=$ATIME;url=$myname"
        > ${LOGIP_LOGDIR}/logip6.log
        show_info "" "$_MN_resetting ${LOGIP_LOGDIR}/logip6.log"
        show_html_footer
    ;;
    download|$_MN_download)
        http_header download "ctype=text/plain;filename=logip6.log"
        cat ${LOGIP_LOGDIR}/logip6.log
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
                        cat ${LOGIP_LOGDIR}/logip6.log | format_output > /tmp/logip6.$$
                    ;;
                    *)
                        cat ${LOGIP_LOGDIR}/logip6.log | sed '1!G;h;$!d' | format_output > /tmp/logip6.$$
                    ;;
                esac
            ;;
            *[0-9]*)
                # is an integer
                n=$FORM_lines
                case $FORM_sort in
                    normal)
                        cat ${LOGIP_LOGDIR}/logip6.log | do_tail $n | format_output > /tmp/logip6.$$
                    ;;
                    *)
                        cat ${LOGIP_LOGDIR}/logip6.log | do_tail $n | sed '1!G;h;$!d' | format_output > /tmp/logip6.$$
                    ;;
                esac
            ;;
        esac
        show_html_header "$_LOGIP_maintitle"
        show_tab_header "$_LOGIP_title" no
        echo "<h2>$_LOGIP_maintitle ($n $_MN_rows)</h2>"
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
        echo "<th>$_LOGIP_date</th>"
        echo "<th>$_LOGIP_typ</th>"
        echo "<th>$_LOGIP_ip</th>"
        echo "<th>$_LOGIP_interface</th>"
        echo "<th>$_LOGIP_name</th>"
        echo "<th>$_LOGIP_is_default_route</th>"
        echo "</tr>"
        cat /tmp/logip6.$$
        rm /tmp/logip6.$$
        echo "</table>"
        show_tab_footer
        show_html_footer
    ;;
esac
