#!/bin/sh
#----------------------------------------------------------------------------
# test-colors.sh - test colors of the HTML-Timetable
# Call like this: ./test-colors.sh > test.htm
#
# Creation:     10.02.2002 tobig
# Last Update:  $Id$
#----------------------------------------------------------------------------

# Change this!
circuits=20

echo "<link REL=STYLESHEET TYPE=\"text/css\" HREF=\"main.css\">"
echo "</head><body>"
echo "<table border=1 cellspacing=2>"

i=1
while [ $i -le $circuits ]
do
  j=1
  echo "<tr>"
  while [ $j -lt $i ]
  do
    echo "<td><table cellspacing=0 cellpadding=0><tr><td id=\"circuit$i\" width=15 align=center>$i</td><td id="circuit$j" width=15 align=center>$j</td></tr></table></td>"
    j=`expr $j + 1`
  done
  echo "</tr>"
  i=`expr $i + 1`
done

echo "</table></body></html>"


