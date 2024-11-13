#!/bin/sh
#------------------------------------------------------------------------------
#                                                                      __FLI4LVER__
# vbox.cgi - display vbox status and calls
# based on the previous main_Vbox.cgi, created by tg/cp
#
# Creation:     2003-06-12  aw
# Last Update:  $Id$
#
#------------------------------------------------------------------------------

# get main helper functions
#set_debug=yes
. /srv/www/include/cgi-helper

case $FORM_action in
    "start*"|"stop*") sec="start" ;;
    del*) sec="delete" ;;
    *) sec="view" ;;
esac

check_rights "vbox" "$sec"
SPOOLPREFX=/var/spool/vbox

# header [refresh]
# 
# Builds the http and html page header and optionally
# inserts a meta refresh tag to automatically reload
# a given URL

header ()
{
    RTIME=$1
    show_html_header "$_MP_vbox" "refresh=$RTIME; url=$myname"
}

# start [num]
#
# Starts the specified vbox

start ()
{
    ttyi_n=$1
    set -- `eval 'echo $VBOX_TTYI_'$ttyi_n`
    user=$2
    case x$REMOTE_USER in
        x$user|x$VBOX_ADMIN_USERNAME|x)
            rm -f /var/run/vbox_stop_$ttyi_n
        ;;
        *)
            return 1
        ;;
    esac
}

# stop [num]
#
# Stops the specified vbox

stop ()
{
    ttyi_n=$1
    set -- `eval 'echo $VBOX_TTYI_'$ttyi_n`
    user=$2
    case x$REMOTE_USER in
        x$user|x$VBOX_ADMIN_USERNAME|x)
            if [ -f /var/run/vboxgetty-ttyI$ttyi_n.pid ]
            then
                echo >/var/run/vbox_stop_$ttyi_n
                kill `cat /var/run/vboxgetty-ttyI$ttyi_n.pid`
                rm -f /var/run/vboxgetty-ttyI$ttyi_n.pid
            fi
        ;;
        *)
            return 1
        ;;
    esac
}

ls2date ()
{
    read zeile
    YEAR=`echo $zeile | cut -d " " -f 5 -`
    DAY=`echo $zeile | cut -d " " -f 3 -`
    if [ $DAY -lt 10 ]
    then
        DAY=`echo 0$DAY`
    fi
    TIME=`echo $zeile | cut -d " " -f 4 -`
    MONTHSUBST='s/Jan/01/g;s/Feb/02/g;s/Mar/03/g;s/Apr/04/g;s/May/05/g;s/Jun/06/g;s/Jul/07/g;s/Aug/08/g;s/Sep/09/g;s/Oct/10/g;s/Nov/11/g;s/Dec/12/g'
    MONTH=`echo $zeile | cut -d " " -f 2 - | sed -e $MONTHSUBST`
    echo $YEAR-$MONTH-$DAY $TIME
}

# Check, if vbox is running
if [ -f /etc/vbox/vboxgetty.conf ]
then
    #get variables
    . /var/run/vbox.conf
    RTIME=9999

    case $FORM_action in
    "")
        header
        echo "<p><a href=\"$myname?action=startall\">$_VBOX_cmd_startall</a>&nbsp;|&nbsp;<a href=\"$myname?action=stopall\">$_VBOX_cmd_stopall</a></p>"
        i='0'
        showbox=
        for i in `seq 1 $VBOX_USER_N`
        do
            set `eval echo '$VBOX_USER_'$i'_USERNAME'`
            USRNAME=$1
            [ "$REMOTE_USER" = "$USRNAME" -o "$REMOTE_USER" = "$VBOX_ADMIN_USERNAME" -o -z "$REMOTE_USER" ] || continue
            SPOOLDIR=$SPOOLPREFX/$USRNAME/incoming
            showbox=yes
            show_tab_header "$USRNAME" no
            echo "<br><table cellpadding=\"0\" cellspacing=\"0\" border=\"0\"><tr><td valign=\"top\">"

            # Show list of users incoming messages

            echo '<table class="normtable">'
            echo "<tr><th>$_VBOX_table_date</th><th>$_VBOX_table_callername</th><th>$_VBOX_table_callerid</th><th>$_VBOX_table_length</th><th>$_VBOX_table_action</th></tr>"

            ROWCOLOR="#e9e9e9"
            MSGLIST=`ls -r $SPOOLDIR`
            for MSG in $MSGLIST
            do
                MSGFILE="$SPOOLDIR/$MSG"
                MSGINFO=`vboxmode $MSGFILE | sed 's/.*\.\.\.: //'`

                case $ROWCOLOR in 
                    "#ffffff") ROWCOLOR="#e9e9e9" ;;
                    *) ROWCOLOR="#ffffff" ;;
                esac

                j='1'
                echo "$MSGINFO" |
                (
                while read MSGVAL
                do
                    case $j in
                        2 ) infodate=`echo $MSGVAL | ls2date` ;;
                        4 ) infolen=`echo "$MSGVAL" | sed -e 's/ seconds//'` ;;
                        5 ) infoname="$MSGVAL" ;;
                        6 ) infocid="$MSGVAL" ;;
                      esac
                    j=`/usr/bin/expr $j + 1`
                done
                  
                echo "<tr bgcolor=\"$ROWCOLOR\">
                      <td align=\"right\" nowrap>$infodate</td>
                      <td nowrap>$infoname</td>
                      <td align=\"right\" nowrap>$infocid</td>
                      <td align=\"right\" nowrap>$infolen&nbsp;&nbsp;</td>
                      <td align=\"center\" nowrap><a href=\"$myname?action=play&amp;msg=$MSG&amp;nr=$i\">$_VBOX_cmd_play</a>&nbsp;&nbsp;<a href=\"$myname?action=info&amp;msg=$MSG&amp;nr=$i\">$_VBOX_cmd_info</a>&nbsp;&nbsp;<a href=\"$myname?action=del&amp;msg=$MSG&amp;nr=$i\">$_VBOX_cmd_delete</a></td>
                      </tr>"
                )
            done

            echo "</table><br>
                  </td><td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td><td valign=\"top\">"

            # Show status of users MSNs
            echo '<table class="normtable">'
            echo "<tr><th>$_VBOX_table_msn</th><th>$_VBOX_table_status</th><th>$_VBOX_table_action</th></tr>"

            for j in `seq $VBOX_TTYI_MIN $VBOX_TTYI_N`
            do
                set `eval echo '$VBOX_TTYI_'$j`
                
                case "$2" in
                "$USRNAME")
                    if [ -f /var/run/vboxgetty-ttyI$j.pid ]
                    then
                        status="$_VBOX_state_running"
                        startstopurl="<a href=\"$myname?nr=$j&amp;action=stop\">$_VBOX_cmd_stop</a>"
                        color="#00FF00"
                    else
                        status="$_VBOX_state_stopped"
                        startstopurl="<a href=\"$myname?nr=$j&amp;action=start\">$_VBOX_cmd_start</a>"
                        color="#FF0000"
                    fi

                    echo "<tr bgcolor=\"#ffffff\">"\
                         "<td bgcolor=\"$color\">$1</td>"\
                         "<td align=\"center\" bgcolor=\"$color\">$status</td>"\
                         "<td align=\"center\">$startstopurl</td></tr>"
                ;;
                esac
            done

            echo "</table>"
            echo "</td></tr></table><br>"
            show_tab_footer
        done
        
        case $showbox in
            yes) ;;
            *) show_error "" "$_VBOX_msg_novbox" ;;
        esac
        show_html_footer
    ;;
    play)
        eval USER='$VBOX_USER_'$FORM_nr'_USERNAME'
        MSG=$SPOOLPREFX/$USER/incoming/$FORM_msg
        if [ -f $MSG ]
        then
            [ -f /usr/bin/vboxbeep ] && vboxbeep > /dev/null 2>&1
            MSGTMP=/tmp/vboxmsg.$$
            vboxtoau <$MSG >$MSGTMP
            set -- `ls -l $MSGTMP`
            length=$5
            echo "Content-type: audio/basic"
            echo "Content-Disposition: attachment; filename=vbox.au"
            echo "Content-Length: $length"
            echo
            cat $MSGTMP
            rm $MSGTMP
        else
            header 3
            show_error "" "$_VBOX_msg_nomessage"
            show_html_footer
        fi
    ;;
    info)
        eval USER='$VBOX_USER_'$FORM_nr'_USERNAME'
        MSG=$SPOOLPREFX/$USER/incoming/$FORM_msg
        if [ -f $MSG ]
        then
            header
            MSGINFO=`vboxmode $MSG`
            show_tab_header "$_VBOX_table_messageinfo" no
            echo "<br><pre>$MSGINFO</pre><br>"
            echo "<p><a href=\"$myname\">$_MN_bck</a></p>"
            show_tab_footer
            show_html_footer
        else
            header 3
            show_error "" "$_VBOX_msg_nomessage"
            show_html_footer
        fi
    ;;
    del)
        eval USER='$VBOX_USER_'$FORM_nr'_USERNAME'
        MSG=$SPOOLPREFX/$USER/incoming/$FORM_msg
        if [ -f $MSG ]
        then
            header
            MSGINFO=`vboxmode $MSG`
            show_tab_header "$_VBOX_table_confirmdelete" no
            echo "<br><pre>$MSGINFO</pre><br>"
            echo "<p>$_VBOX_msg_confirmdelete</p>"
            echo "<p><a href=\"$myname?action=delc&amp;msg=$FORM_msg&amp;nr=$FORM_nr\">$_VBOX_cmd_yes</a>&nbsp;|&nbsp;<a href=\"$myname\">$_VBOX_cmd_no</a></p>"
            show_tab_footer
            show_html_footer
        else
            header 3
            show_error "" "$_VBOX_msg_nomessage"
            show_html_footer
        fi
    ;;
    delc)
        eval USER='$VBOX_USER_'$FORM_nr'_USERNAME'
        MSG=$SPOOLPREFX/$USER/incoming/$FORM_msg
        if [ -f $MSG ]
        then
            header 2
            rm $MSG
            show_info "" "$_VBOX_msg_deleted"
            echo "<p><a href=\"$myname\">$_MN_bck</a>"
            show_html_footer
        else
            header 3
            show_error "" "$_VBOX_msg_nomessage"
            show_html_footer
        fi
    ;;
    start)
        start $FORM_nr
        case $? in
            1) 
                header 3
                show_error "" "$_VBOX_msg_notallowed"
                show_html_footer
            ;;
            *)
                sleep 3
                echo "Location: $myname"
                echo
            ;;
        esac
    ;;
    stop)
        stop $FORM_nr
        case $? in
            1) 
                header 2
                show_error "" "$_VBOX_msg_notallowed"
                show_html_footer
            ;;
            *)
                sleep 3
                echo "Location: $myname"
                echo
            ;;
        esac
    ;;
    "startall" )
        for i in `seq $VBOX_TTYI_MIN $VBOX_TTYI_N`
        do
            start "$i"
        done
        sleep 2
        header 3
        show_info "" "$_VBOX_msg_allstarted"
        echo "<p><a href=\"$myname\">$_MN_bck</a>"
        show_html_footer
    ;;
    "stopall" )
        for i in `seq $VBOX_TTYI_MIN $VBOX_TTYI_N`
        do
            stop "$i"
        done
        sleep 2
        header 3
        show_info "" "$_VBOX_msg_allstopped"
        echo "<p><a href=\"$myname\">$_MN_bck</a>"
        show_html_footer
    ;;
    *)
        echo "Location: $myname"
        echo
    ;;
    esac
else
    header
    show_error "" "<p>$_VBOX_msg_notavailable`grep -q VBOX /bootmsg.txt && echo '<br><br><pre>' ; \
                   grep VBOX /bootmsg.txt ; echo '</pre>'`"
    show_html_footer
fi
