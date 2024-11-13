# $Id$

extra_pcengines_wrap ()
{
    show_tab_header "BIOS" no

    echo "<table class='normtable'>"
    /usr/sbin/pcengines | grep -v Watchdog | sed 's#^#<tr><td>#;s# *: *#</td><td>#;s#$#</td></tr>#;s# #\&nbsp;#g'
    echo "</table>"

    show_tab_footer
}

# _oOo_