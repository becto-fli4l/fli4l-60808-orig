#!/bin/sh

#export DATE="18.03.2007";SCRIPT=${0#/rom}
#export TITLE="freifunkMap"

# import all needed variables
. /var/run/freifunk_register.conf

sp=`echo -e "$FREIFUNK_LAT, $FREIFUNK_LON" | sed s/\ /%20/g`

mapserver="$FREIFUNK_GUI_MAPSERVER"

. ./cgi-bin-pre.sh


if [ -z $mapserver ]; then
	if [ -z $sp ]; then
cat<<EOF
		keine GoogleMapsKoordinaten und kein MapServer konfiguriert 
EOF
	else
cat<<EOF
		kein Mapserver konfiguriert 
EOF
	fi

else

cat<<EOF
<iframe src="$mapserver?sp=$sp&z=16" width="100%" height="620" frameborder="0">
  <p>Ihr Browser kann leider keine eingebetteten Frames anzeigen:
  Sie k&ouml;nnen die eingebettete Seite &uuml;ber den folgenden Verweis
  aufrufen: <a href="$mapserver?sp=$sp&z=16">freifunkMap</a></p>
</iframe>
<br>
<small>&copy; 2007 <a href="http://www.layereight.de">www.layereight.de</a></small>	
EOF

fi 

. ./cgi-bin-post.sh
