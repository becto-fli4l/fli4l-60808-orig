#!/bin/sh
#------------------------------------------------------------------------------
# connections.cgi - show connections to the internet
#
# Creation:     03.02.2001  tg
# Last Update:  $Id$
#
#------------------------------------------------------------------------------

# get main helper functions
. /srv/www/include/cgi-helper

dump_log()
{
    imond-send "imond-log-file" | sed 's/.$//;/^OK/ d;s/^ //'
}

format_output()
{
    htmlspecialchars | while read line
    do
        set -- $line
        test "$#" -lt 13 && continue
        case $1 in
            *unknown*) circ="$_CON_unknown" ;;
            *) circ="$1" ;;
        esac
        echo "<tr><td>$circ</td><td>$2</td><td>$3</td><td>$4</td><td>$5</td>"
        echo "  <td>$6</td><td>$7</td><td>$8</td>"
        # get some smaller numbers
        shift 5
        # detect old logfile format
        case "$#" in
            8)
                echo "  <td>`bytes2read 0 $4`B</td><td>`bytes2read 0 $5`B</td>"
            ;;
            10)
                shift 2
                echo "  <td>`bytes2read $2 $3`B</td><td>`bytes2read $4 $5`B</td>"
            ;;
        esac
        case $6 in
            *unknown*) dev="$_CON_unknown" ;;
            *) dev="$6" ;;
        esac
        echo "  <td>$dev</td><td>$7</td><td>$8</td></tr>"
    done
}
: ${FORM_action:=view}

# Security
check_rights "logs" "$FORM_action"

# Check if imond is running
if [ ! -f /var/run/imond.port ]
then
    show_html_header "$_MN_err"
    show_error "$_MN_err" "imond $_MN_notrun.<br>$_MN_rbfix"
    show_html_footer
    exit 1
fi

ATIME=1
tmp="/tmp/$myname.$$"
port=`cat /var/run/imond.port`

case $FORM_action in
    reset|$_MN_reset)
        show_html_header "$_MN_resetting $_CON_imondlog ..." "refresh=$ATIME;url=$myname"
        show_info "" "$_MN_resetting $_CON_imondlog ... `{ echo "reset-imond-log-file"; echo "quit"; } | netcat localhost $port`"
        show_html_footer
    ;;
    download|$_MN_download)
        http_header download "ctype=text/plain;filename=connections.txt"
        dump_log
        ;;
    *)
        : ${FORM_sort:=reverse}
        case $FORM_sort in reverse) sort="normal" ;; *) sort="reverse" ;; esac
        # show all rows if the "all rows" button is pressed
        case $FORM_showall in "") ;; *) FORM_lines="$_MN_all" ;; esac
        # sanitize no. of rows: show the default of 50 rows if nothing or nonsens (no integer)  is specified
        case $FORM_lines in all|"$_MN_all") ;; *[^0-9]*|"") FORM_lines=50 ;; esac
        case $FORM_lines in
            all|"$_MN_all"|*[^0-9]*)
                # is "all"
                n="$_MN_all"
                case $FORM_sort in 
                    normal)
                        dump_log | format_output > /tmp/connout.$$
                    ;;
                    *)
                        dump_log | sed '1!G;h;$!d' | format_output > /tmp/connout.$$
                    ;;
                esac
            ;;
            *[0-9]*)
                # is an integer
                n=$FORM_lines
                case $FORM_sort in 
                    normal)
                        dump_log | do_tail $n | format_output > /tmp/connout.$$
                    ;;
                    *)
                        dump_log | do_tail $n | sed '1!G;h;$!d' | format_output > /tmp/connout.$$
                    ;;
                esac
            ;;
        esac
        show_html_header "$_MP_cnlst"
        show_tab_header "$_CON_nections" no
        echo "<h2>$_MP_cnlst ($n $_MN_rows)</h2>"
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
        echo "<tr><th>$_CON_circuit</th><th>$_MN_start$_MN_date</th><th>$_MN_start$_MN_time</th><th><a href=\"$myname?lines=$FORM_lines&amp;sort=$sort\">$_MN_end$_MN_date<img alt=\"sort\" src=\"../img/s$sort.gif\"></a></th>"
        echo "<th>$_MN_end$_MN_time</th><th>$_CON_onltime</th><th>$_CON_chtime</th><th>$_CON_charge</th>"
        echo "<th>$_CON_ibytes</th><th>$_CON_obytes</th><th>$_CON_device</th><th>$_CON_chargeint</th>"
        echo "<th>$_CON_chargemin</th></tr>"
        cat /tmp/connout.$$
        rm /tmp/connout.$$
        echo "</table>"
        show_tab_footer
        show_html_footer
    ;;
esac
