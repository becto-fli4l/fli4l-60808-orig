#!/bin/sh
#------------------------------------------------------------------------------
# calls.cgi - show calls loged by telmond
#
# Creation:     03.02.2001  tg
# Last Update:  $Id$
#
# WARNING: This file contains ISO-8859-1 special characters (öäü...).
#          Please make sure to pay attention to the character encoding when
#          editing.
#------------------------------------------------------------------------------

# get main helper functions
. /srv/www/include/cgi-helper
. /etc/telmond_logdir.conf

dump_log ()
{
    cat "$telmond_logdir/telmond.log"
}

format_output ()
{
    tr -d '\r' < /etc/phonebook > $tmp.phbk
    # Build sed script
    cat > $tmp.sed <<-EOF
	s/^/<tr><td>/
	s/$/<\/td><\/tr>/
	s/ \+/<\/td><td>/g
EOF
    # Convert phonebook to sed-expressions and append to script
    sed '/^[^#][0-9*#^=]*=/!d; \
        s/,[^,]*$//; s/|/-/g; s/&/\\\&amp;/g; s/*/\\\*/g; \
        s/</\\\&lt;/g; s/>/\\\&gt;/g; s/ß/\\\&szlig;/g; \
        s/ö/\\\&ouml;/g; s/Ö/\\\&Ouml;/g; \
        s/ä/\\\&auml;/g; s/Ä/\\\&Auml;/g; \
        s/ü/\\\&uuml;/g; s/Ü/\\\&Uuml;/g; \
        s/^\([^=]*\)=\(.*\)/s|>\1<| title="\1">\2<|/;' $tmp.phbk >> $tmp.sed
    # do the transformation
    sed -f $tmp.sed
}

: ${FORM_action:=view}

# Security
check_rights "logs" "$FORM_action"

# Check if telmond is running
if [ ! -f /var/run/telmond.pid -o ! -f /var/run/telmond.port ]
then
    show_html_header "$_MN_err"
    show_error "$_MN_err" "Telmond $_MN_notrun.<br>$_MN_rbfix"
    show_html_footer
    exit 1
fi

tmp="/tmp/$myname.$$"
case $FORM_action in
    reset|$_MN_reset)
        show_html_header "$_MN_resetting $_CALL_telmondlog ..." "refresh=1;url=$myname"
        show_info "" "$_MN_resetting $_CALL_telmondlog ... `echo ""; >"$telmond_logdir/telmond.log"`"
        show_html_footer
    ;;
    download|$_MN_download)
        http_header download "ctype=text/plain;filename=calls.txt"
        dump_log
    ;;
    *)

        # standard sort method is reverse
        : ${FORM_sort:=reverse}
        case $FORM_sort in reverse) sort="normal" ;; *) sort="reverse" ;; esac
        # show all rows if the "all rows" button is pressed
        case $FORM_showall in "") ;; *) FORM_lines="$_MN_all" ;; esac
        # sanitize no. of rows: show the default of 50 rows if nothing or nonsens (no integer)  is specified
        case $FORM_lines in all|"$_MN_all") ;; *[^0-9]*|"") FORM_lines=50 ;; esac
        case $FORM_lines in
            all|"$_MN_all")
                # is "all" or no integer
                n="$_MN_all"
                case $FORM_sort in 
                    normal)
                        dump_log | format_output > $tmp.out
                    ;;
                    *)
                        dump_log | format_output | sed '1!G;h;$!d' > $tmp.out
                    ;;
                esac
            ;;
            *)
                # is an integer
                n=$FORM_lines
                case $FORM_sort in 
                    normal)
                        dump_log | do_tail $n | format_output > $tmp.out
                    ;;
                    *)
                        dump_log | do_tail $n | format_output | sed '1!G;h;$!d' > $tmp.out
                    ;;
                esac
            ;;
        esac
 
        show_html_header "$_MP_calls"
        show_tab_header "$_MP_calls" no
        echo "<h2>$_MP_calls ($n $_MN_rows)</h2>"
        cat <<-EOF
	<form action="$myname" method="GET">
	    <input class="itext" type="text" size="3" maxlength="4" name="lines" value="$FORM_lines">&nbsp;$_MN_rows&nbsp;&nbsp;
	    <input class="sbutton" type="submit" value="$_MN_show">
	    <input class="sbutton" type="submit" name="showall" value="$_MN_BTN_showallrows">
	    <input class="sbutton" type="submit" name="action" value="$_MN_download">
	    <input class="sbutton" type="submit" name="action" value="$_MN_reset">
	    <input type="hidden" name="sort" value="$FORM_sort">
	</form>
	<br>
EOF
        echo '<table class="normtable">'
        echo "<tr><th><a href=\"$myname?lines=$FORM_lines&amp;sort=$sort\">$_MN_date<img alt=\"sort\" src=\"../img/s$sort.gif\"></a></th><th>$_MN_time</th><th>$_CALL_ingnr</th><th>$_CALL_dnr</th></tr>"

        cat $tmp.out

        echo "</table>"
        show_tab_footer
        show_html_footer

        rm $tmp.*
    ;;
esac
