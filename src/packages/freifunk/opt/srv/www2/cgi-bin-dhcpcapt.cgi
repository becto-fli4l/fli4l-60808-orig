#!/bin/sh

#export DATE="3.10.2006";SCRIPT=${0#/rom}
#export TITLE="Hallo
Gast!"
. ./cgi-bin-pre.sh

cat<<EOF
<H1>Hallo
Gast!</H1>
<FORM ACTION="cgi-bin-dhcpcapt.html" METHOD="POST">
EOF

echo '<BASE HREF="http://'$(nvram get wifi_ipaddr)'" />'
if [ "$REQUEST_METHOD" = "POST" ]; then
read QUERY_STRING
fi
if [ -z "$QUERY_STRING" ]; then

cat<<EOF
<TABLE CLASS="shadow0" CELLPADDING="0" CELLSPACING="0"><TR><TD><TABLE CLASS="shadow1" CELLPADDING="0" CELLSPACING="0"><TR><TD><TABLE CLASS="shadow2" CELLPADDING="0" CELLSPACING="0"><TR><TD><TABLE BORDER="0" CLASS="formfixwidth"><TR
TITLE="Angabe eines Spitznamens (kurz, f�r IRC, Chat etc.)">
<TD>
<P>Dies ist ein
Freifunk-Zugriffspunkt mit einem Gastzugang. &Uuml;ber diesen
Gastzugang k&ouml;nnen E-Mails und Web-Seiten kostenfrei abgerufen
werden, wenn die folgenden Bedingungen akzeptiert werden. Weitere
Informationen bietet die <A HREF="cgi-bin-index.html">Hauptseite
dieses Ger&auml;tes</A>.</P></TD>
</TR><TR
TITLE="Angabe eines Spitznamens (kurz, f�r IRC, Chat etc.)">
<TD>&nbsp;</TD>
</TR><TR
TITLE="Angabe eines Spitznamens (kurz, f�r IRC, Chat etc.)">
<TD><INPUT NAME="free" TYPE="CHECKBOX" VALUE="1"> Meine
Daten werden unverschl&uuml;sselt &uuml;bertragen.</TD>
</TR><TR
TITLE="Angabe eines Spitznamens (kurz, f�r IRC, Chat etc.)">
<TD><INPUT NAME="fair" TYPE="CHECKBOX" VALUE="1"> Ich
verhalte mich fair bei der Nutzung dieses Netzwerkes.</TD>
</TR><TR><TD>&nbsp;</TD>
</TR><TR><TD ALIGN="CENTER"><INPUT NAME="post_adress" TITLE="Bitte akzeptieren, wenn Sie mit den Bedingungen einverstanden sind" TYPE="SUBMIT" VALUE="Akzeptieren"></TD>
</TR>
</TABLE></TD></TR></TABLE></TD></TR></TABLE></TD></TR></TABLE></FORM>
EOF

else
if [ "$QUERY_STRING" != "${QUERY_STRING#*free=1}" ] && [ "$QUERY_STRING" != "${QUERY_STRING#*fair=1}" ]; then
echo ok
else

cat<<EOF
<TABLE BORDER="0" CLASS="note"><TR><TD>Bedingungen nicht akzeptiert.</TD>
</TR>
</TABLE>
EOF

fi
fi

. ./cgi-bin-post.sh
