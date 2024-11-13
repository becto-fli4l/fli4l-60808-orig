#!/bin/sh
#------------------------------------------------------------------------------
# index.cgi - startfile for the web administration
#
# Creation:     12.08.2005  hh
# Last Update:  $Id$
#
#------------------------------------------------------------------------------

. /etc/rc.d/bootmsg-filter-common

# get main helper functions
. /srv/www/include/cgi-helper
case $FORM_action in
    delbootmsg)
        rm -f /bootmsg.txt
        reload
    ;;
    *)
        show_html_header
        case x$FORM_link in
        x)
            if [ -f /bootmsg.txt ]
            then
                local regex="$(httpd_get_bootmsg_regex)"
                data="$(grep -v "$regex" /bootmsg.txt)"
                if [ -n "$data" ]
                then
                    show_error "<a href=\"$myname?action=delbootmsg\" >There are messages in /bootmsg.txt (click to delete)</a>" "<pre>`echo "$data" | htmlspecialchars`</pre>"
                    echo "<p>&nbsp;</p>"
                fi
            fi
	    header=
	    for i in /var/log/dumps/*; do
		[ -f $i ] || break
		name=`basename $i`
		set -- `ls -l --full-time $i`
		shift 5
		date="$1 $2"
		if [ ! "$header" ]; then
		    show_tab_header "$_PROB_coredumps" no
		    echo "<table class=\"normtable\"><tr><th>$_PROB_filename</th><th>$_PROB_date</th></tr>"
		    header=yes
		fi
		echo "<tr class=\"$status\"><td>`basename $i`</td><td>$date</td></tr>"
	    done
	    if [ "$header" ]; then
		echo '</table>'
		show_tab_footer
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
