#!/bin/sh
#------------------------------------------------------------------------------
# metalog.cgi - show the metalog files
#
# based upon syslog.cgi by hh
#
# Creation:     2006-10-05 abe
# Last Update:  $Id$
#
#------------------------------------------------------------------------------

# get main helper functions
. /srv/www/include/cgi-helper

metalogdir=`cat /var/run/metalog.dir`

# Functions
get_file_from_target ()
{
    for dx in ${metalogdir}/*/current
    do
        # get the value of the * replacement
        metalogfacility=`dirname $dx`
        metalogfacility=`basename $metalogfacility`
        if [ "$FORM_target" = "$metalogfacility" -a -f "$dx" ]
        then
            metalogfile=$dx
        fi
    done
}

dump_log ()
{
    case $FORM_filter in
        "") cat $metalogfile ;;
        *) grep -v $FORM_filter $metalogfile ;;
    esac
}

format_output ()
{
    htmlspecialchars | while read line
    do
        case "$line" in
            '') ;;
            *)
                set -f
                set -- $line
                case $2$3$4$5 in
                    "Lastoutputrepeated"*)
                        def="<td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td>"
                        sn=0
                        facil="mark"
                        ;;
                    *)
                        def="<td>$1 $2</td><td>$3</td><td>$4</td>"
                        sn=4
                        facil="$4"
                        ;;
                esac

                shift $sn
                echo "<tr class=\"$facil\">$def<td>$*</td></tr>"
                set +f
                ;;
        esac
    done
}

: ${FORM_action:=view}

# Security
check_rights "logs" "$FORM_action"

case $FORM_action in
    download|$_MN_download)
        get_file_from_target
        http_header download "ctype=text/plain;filename=${FORM_target}.log"
        dump_log
        ;;
    *)
        # show all rows if the "all rows" button is pressed
        case $FORM_showall in "") ;; *) FORM_lines="$_MN_all" ;; esac
        # sanitize no. of rows: show the default of 20 rows if nothing or 
        # nonsens (no integer)  is specified
        case $FORM_lines in all|"$_MN_all") ;; *[^0-9]*|"") FORM_lines=20 ;; esac
        for to in ${metalogdir}/*/current
        do
            target=`dirname $to`
            target=`basename $target`
            if [ -f "$to" ]
            then
                case $FORM_target in
                    ""|"$target") 
                        tabs="${tabs}\"$target\" no "
                        FORM_target="$target"
                        ;;
                    *)
                        tabs="${tabs}\"$target\" \"$myname?target=$target&amp;lines=$FORM_lines&amp;\" "
                        ;;
                esac
            fi
        done 
        show_html_header "Metalog" "subtitle=\"($FORM_target)\""
        get_file_from_target
        case $metalogfile in
            "") show_error "$_MN_err" "$_MLOG_nofile ($FORM_target)" ;;
            *)
                case $FORM_lines in
                    all|"$_MN_all")
                        # is "all"
                        n="$_MN_all"
                        dump_log | format_output > /tmp/sysout.$$
                        ;;
                    *)
                        # is an integer
                        n=$FORM_lines
                        dump_log | do_tail $n | format_output > /tmp/sysout.$$
                        ;;
                esac
                eval show_tab_header $tabs
                echo "<h2>$_MLOG_syslogfile: $metalogfile</h2>"
                # Display form to change number of shown rows of logfile
                cat <<-EOF
	<form action="$myname" method="get">
	<input class="itext" type="text" size="3" maxlength="4" name="lines" value="$FORM_lines">&nbsp;$_MN_rows&nbsp;&nbsp;
	<input class="sbutton" type="submit" value="$_MN_show">
	<input class="sbutton" type="submit" name="showall" value="$_MN_BTN_showallrows">
	<input class="sbutton" type="submit" name="action" value="$_MN_download">
	<input type="hidden" name="target" value="$FORM_target">
	</form>
	<br>
EOF
                echo "<table class=\"normtable\"><tr><th>$_MN_date</th><th>$_MN_time</th><th>$_MLOG_syslogfacil</th><th>$_MLOG_message</th></tr>"
                cat  /tmp/sysout.$$
                rm /tmp/sysout.$$ 
                echo '</table>'

                show_tab_footer
                ;;
        esac
        show_html_footer
        ;;
esac

