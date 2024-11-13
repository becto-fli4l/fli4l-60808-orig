#!/bin/sh
#------------------------------------------------------------------------------
# timetable.cgi - shows the circuit timetable in colorful format
#
# Creation:     20.01.2001  tg
# Last Update:  $Id$
#
#------------------------------------------------------------------------------

# get main helper functions
. /srv/www/include/cgi-helper

# Security
check_rights 'status' 'view'

# Check if imond is running
if [ ! -f /var/run/imond.port ]
then
    show_html_header "$_MN_err"
    show_error "$_MN_err" "imond $_MN_notrun"
    show_html_footer
    exit 1
fi

if [ 0$FORM_circuit -gt 0 ]
then
  num=" $FORM_circuit"
  addtext="<p><a href=\"$myname\">$_TT_btov ...</a></p>"
  headtext="$_TT_for Circuit $FORM_circuit"
else
  num=""
  addtext=""
  headtext="$_TT_for $_TT_lcr"
fi

table=`imond-send "timetable$num" | sed '1,10 s/.$//'`
table1=`echo "$table" | sed -n '1,9 p'`
table2=`echo "$table" | sed -n '11,$ p'`
unset table

table1=`echo "$table1" | sed '
  1 s# \{1,5\}#</th><th>#g
  1 s#^</th>#<tr><th></th>#g
  1 s#$#</th></tr>#g
  2d
  3,$ s/ \{1,2\}\([0123456789]\{1,2\}\)/<\/td><td class="circuit\1">\1/g
  3,$ s/^ \?/<tr><td>/
  3,$ s/$/<\/td><\/tr>/'`

table2=`echo "$table2" | sed '
  1 s/^ \?/<tr class="head"><td>/
  2,$ s/^ \{1,2\}\([0123456789]\{1,2\}\)/<tr class="circuit\1"><td><a href="'$myname'?circuit=\1">\1<\/a>/
  s/ \{2,\}/<\/td><td>/g
  s/$/<\/td><\/tr>/
  s/-/\&#8722;/g
  $d '`

show_html_header "$headtext"
show_tab_header "$headtext" no
echo '<table class="normtable">'
echo "$table1"
echo '</table><table class="normtable">'
echo "$table2"
# for debugging only for k in `seq 4 20`; do echo "<tr class=\"circuit$k\"><td>$k</td><td>Circuit $k</td><td>no</td><td>no</td><td>pppoe</td><td>0.0</td><td>15</td></tr>"; done
echo '</table>'
show_tab_footer
echo "$addtext"
show_html_footer
