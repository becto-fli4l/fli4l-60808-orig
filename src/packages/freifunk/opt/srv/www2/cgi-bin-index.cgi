#!/bin/sh

#export DATE="31.10.2006";SCRIPT=${0#/rom}
#export TITLE="Hallo!"
. ./cgi-bin-pre.sh
# import all needed variables
. /var/run/freifunk_register.conf

# customized text on the top
if [ -n "$FREIFUNK_GUI_TEXT_BEFORE_FILE" ];
then 

	if [ -f "/srv/www2/$FREIFUNK_GUI_TEXT_BEFORE_FILE" ];
	then 
		cat /srv/www2/$FREIFUNK_GUI_TEXT_BEFORE_FILE
	else
	 	FREIFUNK_GUI_SHOW_DEFTEXTTOP='yes'
	fi
fi

# default text on the top
if [ "$FREIFUNK_GUI_SHOW_DEFTEXTTOP" = 'yes' ]
then 


cat<< EOF
<H1>Hallo!</H1>
<P>Dies ist ein Freifunk-Zugriffspunkt (neudeutsch: WLAN-Access-Point), auf dem
die fli4l Router Software  mit dem Freifunk-Plugin OPT_FREIFUNK
l&auml;uft. Technische Details &uuml;ber diesen Access-Point sind auf der
<A HREF="cgi-bin-status.cgi">Statusseite</A> nachzulesen. Informationen &uuml;ber
das Freifunk-Projekt finden sich im Internet unter
<A HREF="http://www.freifunk.net/">http://www.freifunk.net/</A>.</P>
<DIV><A HREF="http://fli4l.de/"><IMG ALIGN="right" ALT="fli4l Logo" BORDER="0" HEIGHT="31" SRC="images/fli4l.gif" WIDTH="100" TITLE="fli4l Logo"></A></DIV>
<P><B>Hinweis</B>:OPT_FREIFUNK basiert auf der ganz
hervorragenden Linux-Distribution <A HREF="http://fli4l.de/">fli4l</A>.
</P>
EOF

fi

# calculate the picture properties here. Default is "images/intro.jpg"

if [ "$FREIFUNK_GUI_SHOW_DEFPIC" = 'yes' ]
then 
cat<< EOF
<CENTER>
<IMG ALT="Freifunk" BORDER="0" SRC="images/intro.jpg" TITLE="Freifunk">
<BR CLEAR="all">
</CENTER>
EOF
fi

if [ -f "/srv/www2/$FREIFUNK_GUI_PIC_FILE" ];
then 

cat<< EOF
<CENTER>
<IMG ALT="$FREIFUNK_GUI_PIC_ALT" BORDER="0" SRC="$FREIFUNK_GUI_PIC_FILE" TITLE="$FREIFUNK_GUI_PIC_ALT">
<BR CLEAR="all">
</CENTER>
EOF

fi

# Links header
cat<< EOF
<H2>
Links</H2>
<UL>
EOF

# customized links 

idx=1
  	while [  "$idx" -le "$FREIFUNK_GUI_LINK_N" ]
    		do
        	# Inhalt der Variable
        	eval temp='$FREIFUNK_GUI_LINK_'$idx
		eval temp2='$FREIFUNK_GUI_LINK_'$idx'_URL'
		eval temp3='$FREIFUNK_GUI_LINK_'$idx'_TARGET'
	       	# index um eins erhoehen
        	idx=`/usr/bin/expr $idx + 1`



		echo "<LI><A HREF=\"$temp2\" TARGET=\"$temp3\">$temp</A></LI>"
	done

# default links
if [ "$FREIFUNK_GUI_SHOW_DEFLINKS" = 'yes' ]
then 

cat<< EOF
<LI><A HREF="http://www.fli4l.de"  TARGET="_blank">fli4l - Router Homepage</A></LI>
<LI><A HREF="http://olsr.freifunk.net/" TARGET="_blank">OLSR Seite im
freifunk.net</A></LI>
<LI><A HREF="http://olsrexperiment.de/" TARGET="_blank">OLSR-Experiment (Berliner
Forum)</A></LI>
<LI><A HREF="http://olsrexperiment.de/freifunk/" TARGET="_blank">IP
Vergabe (Berliner Adressverwaltung)</A></LI>
<LI><A HREF="cgi-bin-dev-zero.cgi">Endlos-Download aus
/dev/zero</A></LI>
EOF

fi

cat<< EOF
</UL>
EOF

# customized text on bottom


if [ -n "$FREIFUNK_GUI_TEXT_AFTER_FILE" ];
then 


	if [ -f "/srv/www2/$FREIFUNK_GUI_TEXT_AFTER_FILE" ];
	then 
		cat /srv/www2/$FREIFUNK_GUI_TEXT_AFTER_FILE
	else
	 	FREIFUNK_GUI_SHOW_DEFTEXTDOWN='yes'
	fi
fi

# default text on bottom

if [ "$FREIFUNK_GUI_SHOW_DEFTEXTDOWN" = 'yes' ]
then 

cat<< EOF
<P><B>Hinweis</B>:
Der Abruf von Internet-Seiten &uuml;ber das Freifunk-Netzwerk ist an
technische und organisatorische Bedingungen gekn&uuml;pft. M&ouml;glicherweise
funktionieren daher die Verkn&uuml;pfungen zu Internet-Seiten nicht.</P>
EOF

fi

. ./cgi-bin-post.sh
