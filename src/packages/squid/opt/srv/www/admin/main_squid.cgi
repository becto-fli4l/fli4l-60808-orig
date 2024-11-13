#!/bin/sh
#------------------------------------------------------------------------------
# squid.cgi - main_squid.cgi - Main OPT CGI for squid
#
# OPT basiert auf - OPT_SQUID 2.1.7 - 1.1
#                   von Hermann Strassner (hermann.strassner@web.de)
#
#                 - OPT_NEWSQUID 2.0
#                   von Dominik Egert (fli4l@killus.net)
#
# Bearbeitet von: - Ingo Winiarski (iwiniarski@gmx.de)
#
# Last update:  $Id$
#
#------------------------------------------------------------------------------

myname=`basename $0`

. /srv/www/include/cgi-helper

show_html_header "OPT_Squid"

echo '<script language="JavaScript">'
echo '	function ow(lo){'
echo '		windowprops = "width=" + (screen.availWidth - 10) + ",height=200,location=no,menubar=no,personalbar=no,resizable=no,locationbar=no,scrollbars=yes,directories=no,statusbar=no,toolbar=yes";'
echo '		var squidlogshow = window.open(lo, 'squidlogshow', windowprops);'
echo '	}'
echo '</script>'

echo "<br><h2>squid is running with PID: "
cat /var/run/squid.pid
echo "</h2><br>"

echo "<ul>"
echo "  <li><a href=cachemgr.cgi>CacheManager</a></li>"
echo "  <li>View Log Files (live): <a href=\"javascript:ow('squid_showlog.cgi?access.log')\">access.log</a> | "
echo "      <a href=\"javascript:ow('squid_showlog.cgi?cache.log')\">cache.log</a></li>"
echo "  <li>View complete Log Files: <a href=\"squid_catlog.cgi?access.log\">access.log</a> | "
echo "      <a href=\"squid_catlog.cgi?cache.log\">cache.log</a></li>"
echo "</ul>"
echo "<br>"


echo "<table border=0 width=100% bgcolor=#000000>"
echo "  <tr>"
echo "    <td width=100%>"
echo "<font color=white><pre>"
echo "squid direct output:"
echo "===================="
echo ""
cat /etc/squid/squid.out
echo "</pre></font>"
echo "</td>"
echo "  </tr>"
echo "</table>"

show_html_footer
