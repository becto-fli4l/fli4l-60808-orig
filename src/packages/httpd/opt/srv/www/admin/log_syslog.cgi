#!/bin/sh
#------------------------------------------------------------------------------
# syslog.cgi - show the syslog-file
#
# Creation:     24.02.2001  tg
# Last Update:  $Id$
#
#------------------------------------------------------------------------------

# get main helper functions
. /srv/www/include/cgi-helper

# log destination for all messages that do not match any configured selector
global_protocol_file=/var/log/messages

# Functions
get_file_from_target ()
{
    syslogfile=
    echo "$FORM_target" | grep -q '^[[:digit:]]\+$' || return
    [ ${FORM_target} -lt $SYSLOG_DEST_N ] || return
    eval syslogfile=\$SYSLOG_DEST_${FORM_target}_LOGFILE
    eval selectors=\$SYSLOG_DEST_${FORM_target}_SELECTORS
}

dump_log ()
{
    if [ -f "$syslogfile" ]
    then
        case $FORM_filter in
            "") cat $syslogfile ;;
            *) grep -v $FORM_filter $syslogfile ;;
        esac
    fi
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
                def="<td>$1 $2</td><td>$3</td><td>$4</td>"
                case $5$6$7 in
                    "--MARK--"*|"lastmessagerepeated"*)
                        def="$def<td>&nbsp;</td>"
                        sn=4
                        facil="mark"
                    ;;
                    kernel*)
                        def="$def<td>$5&nbsp;$6</td>"
                        sn=6
                        facil="$6"
                    ;;
                    *)
                        def="$def<td>$5</td>"
                        sn=5
                        facil="$5"
                    ;;
                esac
                shift $sn
                echo "<tr class=\"$facil\">$def<td>$*</td></tr>"
                set +f
            ;;
        esac
    done
}

parse_syslog_conf()
{
    SYSLOG_DEST_N=0
    local global_protocol_file_found= selectors logfile
    while read selectors logfile
    do
        # ignore everything that is not a file (pipes, devices, remote hosts...)
        case $logfile in
        /*)
            [ -e "$logfile" -a ! -f "$logfile" ] && continue
            ;;
        *)
            continue
            ;;
        esac

        [ "$logfile" = "$global_protocol_file" ] && global_protocol_file_found=1
        eval SYSLOG_DEST_${SYSLOG_DEST_N}_SELECTORS=\$selectors
        eval SYSLOG_DEST_${SYSLOG_DEST_N}_LOGFILE=\$logfile
        SYSLOG_DEST_N=$((SYSLOG_DEST_N+1))
    done < /etc/syslog.conf

    if [ -z "$global_protocol_file_found" -a -f "$global_protocol_file" ]
    then
        eval SYSLOG_DEST_${SYSLOG_DEST_N}_SELECTORS=
        eval SYSLOG_DEST_${SYSLOG_DEST_N}_LOGFILE=\$global_protocol_file
        SYSLOG_DEST_N=$((SYSLOG_DEST_N+1))
    fi
}

: ${FORM_action:=view}

# Security
check_rights "logs" "$FORM_action"

if [ ! -f /etc/syslog.conf ]
then
    show_html_header "$_MN_err"
    show_error "$_MN_err" "$_SLOG_nosyslog"
    show_html_footer
else
    parse_syslog_conf

    case $FORM_action in
    reset|$_MN_reset)
        get_file_from_target
        case $syslogfile in
            "") ;;
            *)
                > $syslogfile
            ;;
        esac
        echo "Location: $myname?target=$FORM_target"
        echo
    ;;
    download|$_MN_download)
        get_file_from_target
        case $syslogfile in
            "") ;;
            *)
                if [ -f "$syslogfile" ]
                then
                    http_header download "ctype=text/plain;filename=`basename $syslogfile`"
                    dump_log
                fi
            ;;
        esac
    ;;
    *)
        # standard sort method is normal
        : ${FORM_sort:=normal}
        case $FORM_sort in
            reverse)
                invsort="normal"
                ;;
            *)
                FORM_sort="normal" # make sure we don't have to escape $FORM_sort
                invsort="reverse"
                ;;
        esac
        # show all rows if the "all rows" button is pressed
        case $FORM_showall in "") ;; *) FORM_lines="$_MN_all" ;; esac
        # sanitize no. of rows: show the default of 50 rows if nothing or nonsens (no integer)  is specified
        case $FORM_lines in all|"$_MN_all") ;; *[^0-9]*|"") FORM_lines=50 ;; esac
        for i in $(seq 0 $((SYSLOG_DEST_N-1)))
        do
            eval target=\$SYSLOG_DEST_${i}_SELECTORS
            eval to=\$SYSLOG_DEST_${i}_LOGFILE
            case $FORM_target in
            ""|"$i") 
                tabs="${tabs}\"$_SLOG_syslogfile: $to\" no "
                FORM_target="$i"
            ;;
            *)
                tabs="${tabs}\"$_SLOG_syslogfile: $to\" \"$myname?target=$i&amp;lines=$FORM_lines&amp;sort=$FORM_sort\" "
            ;;
            esac
        done
        show_html_header "Syslog"
        get_file_from_target
        case $syslogfile in
            "") show_error "$_MN_err" "$_SLOG_nofile (Index $(echo "$FORM_target" | htmlspecialchars))" ;;
            *)
                case $FORM_lines in
                    all|"$_MN_all")
                        # is "all"
                        n="$_MN_all"
                        case $FORM_sort in 
                            normal)
                                dump_log | format_output > /tmp/sysout.$$
                            ;;
                            *)
                                dump_log | sed '1!G;h;$!d' | format_output > /tmp/sysout.$$
                            ;;
                        esac
                    ;;
                    *)
                        # is an integer
                        n=$FORM_lines
                        case $FORM_sort in 
                            normal)
                                dump_log | do_tail $n | format_output > /tmp/sysout.$$
                            ;;
                            *)
                                dump_log | do_tail $n | sed '1!G;h;$!d' | format_output > /tmp/sysout.$$
                            ;;
                        esac
                    ;;
                esac
                eval show_tab_header $tabs
                echo "<h2>$_SLOG_syslogfile: $syslogfile ($(echo "$selectors" | htmlspecialchars))</h2>"
                # Display form to change number of shown rows of logfile
                cat <<-EOF
	<form action="$myname" method="GET">
	    <input class="itext" type="text" size="3" maxlength="4" name="lines" value="$FORM_lines">&nbsp;$_MN_rows&nbsp;&nbsp;
	    <input class="sbutton" type="submit" value="$_MN_show">
	    <input class="sbutton" type="submit" name="showall" value="$_MN_BTN_showallrows">
	    <input class="sbutton" type="submit" name="action" value="$_MN_download">
	    <input class="sbutton" type="submit" name="action" value="$_MN_reset">
	    <input type="hidden" name="target" value="$FORM_target">
	    <input type="hidden" name="sort" value="$FORM_sort">
	    <br>
	</form>
EOF
                echo "<table class=\"normtable\"><tr><th><a href=\"$myname?target=$FORM_target&amp;lines=$FORM_lines&amp;sort=$invsort\">$_MN_date<img src=\"../img/s$invsort.gif\" alt=\" (sort $invsort)\"></a></th><th>$_MN_time</th><th>$_SLOG_loghost</th><th>$_SLOG_syslogfacil</th><th>$_SLOG_message</th></tr>"
                cat  /tmp/sysout.$$
                rm /tmp/sysout.$$ 
                echo '</table>'

                show_tab_footer
            ;;
        esac
        show_html_footer
    ;;
    esac
fi
