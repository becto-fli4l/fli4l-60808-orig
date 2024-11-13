#!/bin/sh
#------------------------------------------------------------------------------
# opt/files/srv/www/admin/umts.cgi                                 __FLI4LVER__
#
# Creation:     Hans Kraus
# Last Update:  $Id$
#------------------------------------------------------------------------------

. /srv/www/include/cgi-helper

# Security
check_rights "umts" "view"

show_html_header "UMTS"

val=`get_umts_csq webif`
signalval=`echo $val | cut -d "," -f 1`
error=`echo $val | cut -d "," -f 2`

if [ x$signalval = x -o x$signalval = x99 ] ; then
	signal="keine Angabe"
else
	signal="$signalval (`expr $signalval \* 2 - 113`dbm)"
fi

if [ x$error = x -o x$error = x99 ] ; then
	error="keine Angabe"
fi 

show_tab_header "Empfang" no

echo "<table border='1'>
   <tr><th>Status</th></tr>
   <tr align='center'><td> Signalqualit&auml;t: $signal </td></tr>
   <tr align='center'><td> Bitfehlerrate: $error </td></tr>
</table>"

show_tab_footer

echo '<br>'

show_tab_header "Signalqualit&auml;t" no
echo "<table border='1' cellspacing='0px' cellpadding='0px'>"
echo "<tr><td style='width: 500px; height: 20px;'>"

if echo $signalval | grep -q "^[0-9]\{1,2\}$" && [ "$signalval" -le 31 ]
then
   width=$(expr $signalval \* 500 / 31)
   echo "<div style='position: relative; top: -10px'><div style='position:absolute; clip:rect(auto, ${width}px, auto, auto);'><img src='/img/gradient.png' /></div></div>"
else
   echo "<div style='position: relative; top: -10px'><span style='position: absolute; top: auto; left: 20px; width: 100%; font-weight: bold;'>Ung&uuml;ltiger Wert</span></div>"
fi

echo '</td></tr>'
echo '</table>'
show_tab_footer

echo '<br>'

show_tab_header "Adapter" no

echo "<table border='1'>
   <tr align='center'><th colspan='3'>aktuelle Einstellungen</th></tr>
   <tr><td>Befehl</td><td>Wert</td><td>Bedeutung</td></tr>"

sed -e '{/^\(AT\&V\|OK\)$/d};s/; /\n/g;s/;//g;s/\\/\\\\/g' /tmp/umts.settings |
while read line; do 
  syntax=$(echo $line | cut -d":" -f1 | sed -e 's/\\/[\\]/g')
  [ -z $syntax ] || echo $line:$(grep "^$syntax:" /usr/share/at_syntax|cut -d: -f2-)
done | 
sed -e "s#\(..*\)#   <tr><td>&</td></tr>#g;s#:#</td><td>#g;s#<td>\(\| \)</td>#<td>\&nbsp;</td>#g"

echo "</table>"

show_tab_footer
show_html_footer
