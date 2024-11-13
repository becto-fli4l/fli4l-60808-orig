#!/bin/sh
#------------------------------------------------------------------------------
# index.cgi - startfile for the web administration
#
# Creation:     12.08.2005  hh
# Last Update:  $Id$
#
#------------------------------------------------------------------------------

# get main helper functions
. /srv/www/include/cgi-helper
. /etc/rc.d/bootmsg-filter-common

case $FORM_action in
    logout)
        case "x$FORM_time" in
        x)
            echo "Location: $myname?action=logout&time=`date +%s`"
            echo
        ;;
        *)
            ta=`date +%s`
            td=`expr $ta - $FORM_time`
            # only send 401 if time difference between logout request and entered password is less or equal 3 seconds
            case $td in
            0|1|2|3)
                echo 'Status: 401 Unauthorized'
                echo 'WWW-Authenticate: Basic realm="admin"'
                echo
            ;;
            *)
                echo "Location: $myname"
                echo
            ;;
            esac
        ;;
        esac
    ;;
    delbootmsg)
        rm -f /bootmsg.txt
        reload
    ;;
    *)
        case x$FORM_link in
        x)
            show_html_header "$_MN_welc"
            cat <<-EOF
	    <h2 style="margin: 0px;">$_MN_welc</h2>
	    <h3>$_MN_lgot</h3>
            <p>&nbsp;</p>
EOF
            if [ -f /bootmsg.txt ]
            then
                regex="$(httpd_get_bootmsg_regex)"
                data="$(grep -v "$regex" /bootmsg.txt)"
                if [ -n "$data" ]
                then
                    show_error "<a href=\"$myname?action=delbootmsg\" >There are messages in /bootmsg.txt (click to delete)</a>" "<pre>`echo "$data" | htmlspecialchars`</pre>"
                    echo "<p>&nbsp;</p>"
                fi
            fi
        ;;
        *)
            show_html_header
            echo "<iframe src=\"$(echo "$FORM_link" | htmlspecialchars quot)\" id=\"iframe\"></iframe>"
        ;;
        esac
        show_html_footer
;;
esac
