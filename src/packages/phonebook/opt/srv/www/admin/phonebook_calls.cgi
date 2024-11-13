#!/bin/sh
#####################################################################
#   File:         /srv/www/admin/phonebook_calls.cgi                #
#   Creation:     03.02.2001 tg                                     #
#   Modification: 20.06.2007 jk                                     #
#   Last Update:  $Id$                                              #
#####################################################################
# phonebook_calls.cgi - show calls logged by telmond                #
# angepaﬂte calls.cgi f¸r die Verwendung mit OPT_Phonebook          #
#####################################################################

# get main helper functions
. /srv/www/include/cgi-helper

dump_log()
{
    imond-send "client-ip $REMOTE_ADDR" "telmond-log-file" | sed 's/.$//;/^OK/ d;s/^ //'
}

: ${FORM_action:=view}

# Security
check_rights "logs" "$FORM_action"

# Check if imond and telmond are running
if [ ! -f /var/run/telmond.pid -o ! -f /var/run/imond.port ]
then
    show_html_header "$_MN_err"
    show_error "$_MN_err" "Imond $_MN_or Telmond $_MN_notrun.<br>$_MN_rbfix"
    show_html_footer
    exit 1
fi

tmp="/tmp/$myname.$$"
case $FORM_action in
    reset|$_MN_reset)
        show_html_header "$_MN_resetting $_CALL_telmondlog ..." "refresh=1;url=$myname"
        show_info "" "$_MN_resetting $_CALL_telmondlog ... `imond-send "client-ip $REMOTE_ADDR" "reset-telmond-log-file"`"
        show_html_footer
    ;;
    download|$_MN_download)
        http_header download "ctype=text/plain;filename=calls.txt"
        dump_log
    ;;
    *)
        # Build sed script
        cat > $tmp.sed <<-EOF
	s/^/<tr><td>/
	s/$/<\/td><\/tr>/
	s/ \+/<\/td><td>/g
EOF

        # Convert phonebook to sed-expressions and append to script
        #ƒnderung bei jk
        #original# sed '/^#/d; /^[^=]*$/d; s/^/s|>/; s/&/\\\&/g; s/*/\\\*/g; s/=/<|>/; s/,[^,]*$//; s/$/<|g/' /etc/phonebook >> $tmp.sed
        sed -e "s|\([a-zA-Zƒ÷‹‰ˆ¸ﬂ][a-zA-Zƒ÷‹‰ˆ¸ﬂ\._-]* [0-9][0-9]*[;,] [0-9][0-9][0-9][0-9][0-9] [a-zA-Zƒ÷‹‰ˆ¸ﬂ][a-zA-Zƒ÷‹‰ˆ¸ﬂ\._ -]*\)|<a href='http://maps.google.de/maps?f=q\&hl=de\&q=\1\&btnG=Adressen+suchen'>\1</a>|" -e '/^#/d; /^[^=]*$/d; s/^/s|>/; s/&/\\\&/g; s/*/\\\*/g; s/=/<|>/; s/,[^,]*$//; s/$/<|g/ '  /etc/phonebook >> $tmp.sed


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
                        dump_log | sed -f $tmp.sed > $tmp.out
                    ;;
                    *)
                        dump_log | sed -f $tmp.sed | sed '1!G;h;$!d' > $tmp.out
                    ;;
                esac
            ;;
            *)
                # is an integer
                n=$FORM_lines
                case $FORM_sort in 
                    normal)
                        dump_log | sed -e :a -e '$q;N;'`expr $n + 1`',$D;ba' | sed -f $tmp.sed > $tmp.out
                    ;;
                    *)
                        dump_log | sed -e :a -e '$q;N;'`expr $n + 1`',$D;ba' | sed -f $tmp.sed | sed '1!G;h;$!d' > $tmp.out
                    ;;
                esac
            ;;
        esac
        

        # just for debugging: cat /var/log/telmond.log | sed -f $tmp.sed > $tmp.out

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
