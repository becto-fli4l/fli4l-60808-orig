#!/bin/sh

# export DATE="3.10.2006";SCRIPT=${0#/rom}
# export TITLE="Kontakt"
. ./cgi-bin-pre.sh

# import important variables
. /var/run/freifunk_register.conf
 
cat<<EOF
<H1>Kontakt</H1>
<P>Der Administrator dieses Access-Points hat die folgenden Kontakt-Daten eingetragen:</P>
<TABLE CLASS="shadow0" CELLPADDING="0" CELLSPACING="0"><TR><TD><TABLE CLASS="shadow1" CELLPADDING="0" CELLSPACING="0"><TR><TD><TABLE CLASS="shadow2" CELLPADDING="0" CELLSPACING="0"><TR><TD><TABLE BORDER="1" CELLPADDING="1" CELLSPACING="0" CLASS="form" RULES="rows" WIDTH="448"><TR><TD>Spitzname:&nbsp;</TD>
<TD>$FREIFUNK_GUI_NICKNAME</TD>
</TR><TR><TD>Name:&nbsp;</TD><TD>$FREIFUNK_GUI_NAME</TD>
</TR><TR><TD>E-Mail:&nbsp;</TD><TD>$FREIFUNK_GUI_EMAIL</TD>
</TR><TR><TD>Telefon:&nbsp;</TD><TD>$FREIFUNK_GUI_FONE</TD>
</TR><TR><TD>Standort:&nbsp;</TD>
<TD>$FREIFUNK_GUI_LOCATION</TD>
</TR><TR><TD>GPS:&nbsp;</TD><TD>$FREIFUNK_LAT, $FREIFUNK_LON</TD>
</TR><TR><TD>Notiz:&nbsp;</TD><TD>$FREIFUNK_GUI_NOTICE</TD>
</TR>
</TABLE></TD></TR></TABLE></TD></TR></TABLE></TD></TR></TABLE>
EOF

. ./cgi-bin-post.sh
