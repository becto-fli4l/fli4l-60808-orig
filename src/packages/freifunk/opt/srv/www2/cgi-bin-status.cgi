#!/bin/sh

# export DATE="12.4.2006";SCRIPT=${0#/rom}
# export TITLE="Status: OLSR"
. ./cgi-bin-pre.sh

. /var/run/freifunk_register.conf

WLDEV=$(iwconfig 2>/dev/null | sed -n '/^wlan[0-9]/s/ .*$//p')


if [ "$REQUEST_METHOD" = "POST" ]; then
read QUERY_STRING
fi
if [ "$QUERY_STRING" != "${QUERY_STRING#*post_olsr=}" ]; then
if [ "$FREIFUNK_GUI_SHOW_OLSRINFO" = "yes" ]; then
cat<<EOF
<H1>Status: OLSR</H1>
<FORM ACTION="cgi-bin-status.cgi" METHOD="POST"><TABLE CLASS="shadow0" CELLPADDING="0" CELLSPACING="0"><TR><TD><TABLE CLASS="shadow1" CELLPADDING="0" CELLSPACING="0"><TR><TD><TABLE CLASS="shadow2" CELLPADDING="0" CELLSPACING="0"><TR><TD><TABLE BORDER="1" CELLPADDING="0" CELLSPACING="0" CLASS="form"><TR><TD>
EOF

cat<<EOF
<INPUT NAME="post_overview" TITLE="Allgemeine Eigenschaften des Ger&auml;tes in einer &Uuml;bersicht anzeigen" TYPE="SUBMIT" VALUE="&Uuml;bersicht">&nbsp;&nbsp;&nbsp;
EOF


if [ "$FREIFUNK_GUI_SHOW_ROUTING" = "yes" ]; then
cat<<EOF
<INPUT NAME="post_route" TITLE="Die aktuelle Routingtabelle des Ger&auml;tes anzeigen" TYPE="SUBMIT" VALUE="Routen">&nbsp;
EOF
fi

if [ "$FREIFUNK_GUI_SHOW_WLANSCAN" = "yes" ]; then
cat<<EOF
<INPUT NAME="post_scan" TITLE="Scan nach empfangbaren WLAN-Stationen ausf&uuml;hren" TYPE="SUBMIT" VALUE="WLAN Scan">&nbsp;
EOF
fi

if [ "$FREIFUNK_GUI_SHOW_OLSRINFO" = "yes" ]; then
cat<<EOF
<INPUT NAME="post_olsr" TITLE="OLSR Statusseite anzeigen" TYPE="SUBMIT" VALUE="OLSR-Info">
EOF
fi

cat<<EOF
</TD>
</TR>

<TR><TD>&nbsp;</TD>
</TR><TR><TD>
EOF

wget -q -O - http://127.0.0.1:2006/ | sed -ne'
/^Table:/,/^$/{
s#^Table: \(.*\)#<H2>\1</H2><\TABLE BORDER="1" CELLSPACING="0" CELLPADDING="0" WIDTH="100%">#
:n
p
n
s#[0-9]\+\.[0-9]\+\.[0-9]\+\.[0-9]\+#<A HREF="http://&/">&</A>#g
s/		/	-	/g
s/	$//
s#	#</TD><TD>#g
s#.\+#<TR><TD>&</TD></TR>#
s/./&/
tn
c\
<\/TABLE>
p
}
'

cat<<EOF
</TD>
</TR>
</TABLE></TD></TR></TABLE></TD></TR></TABLE></TD></TR></TABLE>
</FORM>
EOF

fi

elif [ "$QUERY_STRING" != "${QUERY_STRING#*post_route=}" ]; then
if [ "$FREIFUNK_GUI_SHOW_ROUTING" = "yes" ]; then

cat<<EOF
<H1>Status: Routen</H1>
<FORM ACTION="cgi-bin-status.cgi" METHOD="POST"><TABLE CLASS="shadow0" CELLPADDING="0" CELLSPACING="0"><TR><TD><TABLE CLASS="shadow1" CELLPADDING="0" CELLSPACING="0"><TR><TD><TABLE CLASS="shadow2" CELLPADDING="0" CELLSPACING="0"><TR><TD><TABLE BORDER="1"
CELLPADDING="0" CELLSPACING="0" CLASS="form"><TR><TD>
EOF

cat<<EOF
<INPUT NAME="post_overview" TITLE="Allgemeine Eigenschaften des Ger&auml;tes in einer &Uuml;bersicht anzeigen" TYPE="SUBMIT" VALUE="&Uuml;bersicht">&nbsp;&nbsp;&nbsp;
EOF

if [ "$FREIFUNK_GUI_SHOW_ROUTING" = "yes" ]; then
cat<<EOF
<INPUT NAME="post_route" TITLE="Die aktuelle Routingtabelle des Ger&auml;tes anzeigen" TYPE="SUBMIT" VALUE="Routen">&nbsp;
EOF
fi

if [ "$FREIFUNK_GUI_SHOW_WLANSCAN" = "yes" ]; then
cat<<EOF
<INPUT NAME="post_scan" TITLE="Scan nach empfangbaren WLAN-Stationen ausf&uuml;hren" TYPE="SUBMIT" VALUE="WLAN Scan">&nbsp;
EOF
fi

if [ "$FREIFUNK_GUI_SHOW_OLSRINFO" = "yes" ]; then
cat<<EOF
<INPUT NAME="post_olsr" TITLE="OLSR Statusseite anzeigen" TYPE="SUBMIT" VALUE="OLSR-Info">
EOF
fi

cat<<EOF
</TD>
</TR>

<TR><TD>&nbsp;</TD>
</TR><TR><TD>
EOF

# check for FLI4L version
#if [ -f /sbin/ip ]
#    then
#{
#echo '<''TABLE BORDER="0" CELLPADDING="0" CELLSPACING="0">'
#ip route show | sed -e'
#s#^\([^ ]\+\)\(.*\)#<TR>\
#<TD STYLE="padding:0 2 0 2;font-size:9px;">\1</TD>\
#<TD STYLE="padding:0 2 0 2;font-size:9px;">\2</TD>\
#</TR>
#s#\([0-9]\+\.[0-9]\+\.[0-9]\+\.[0-9]\+\)\([^/]\)#<A HREF="http://\1/">\1</A>\2#g
#'
#echo '<''/TABLE>'
#}
#else
#{
netstat -r | awk '

function td(s) {
print "<TD STYLE=\"padding:0 2 0 2;font-size:9px;\">"s"</TD>"

#}

function ip(s) {
if(!/\//&&s!="default"&&s!="Kernel"&&s!="Destination"&&s>"104.0.0.0"&&s<"105.") {
td(sprintf("<A HREF=\"http://%s/\">%s</A>", s, s))
}
else {
td(s)
}
}
BEGIN {
print "<""TABLE BORDER=\"0\" CELLPADDING=\"0\" CELLSPACING=\"0\">"
}
{
print "<TR></TR>"
ip($1)
s=""
for(i=2;i<=NF;i++) s=s" "$i
td(s)
print "</TR>"
}
END {
print "<""/TABLE>"
}'
}

cat<<EOF
</TD>
</TR>
</TABLE></TD></TR></TABLE></TD></TR></TABLE></TD></TR></TABLE>

</FORM>
EOF
fi

elif [ "$QUERY_STRING" != "${QUERY_STRING#*post_scan=}" ]; then
if [ "$FREIFUNK_GUI_SHOW_WLANSCAN" = "yes" ]; then

cat<<EOF
<H1>Status: WLAN Scan</H1>
<FORM ACTION="cgi-bin-status.cgi" METHOD="POST"><TABLE CLASS="shadow0" CELLPADDING="0" CELLSPACING="0"><TR><TD><TABLE CLASS="shadow1" CELLPADDING="0" CELLSPACING="0"><TR><TD><TABLE CLASS="shadow2" CELLPADDING="0" CELLSPACING="0"><TR><TD><TABLE BORDER="1" CELLPADDING="0" CELLSPACING="0" CLASS="form"><TR><TD COLSPAN="7">
EOF

cat<<EOF
<INPUT NAME="post_overview" TITLE="Allgemeine Eigenschaften des Ger&auml;tes in einer &Uuml;bersicht anzeigen" TYPE="SUBMIT" VALUE="&Uuml;bersicht">&nbsp;&nbsp;&nbsp;
EOF

if [ "$FREIFUNK_GUI_SHOW_ROUTING" = "yes" ]; then
cat<<EOF
<INPUT NAME="post_route" TITLE="Die aktuelle Routingtabelle des Ger&auml;tes anzeigen" TYPE="SUBMIT" VALUE="Routen">&nbsp;
EOF
fi

if [ "$FREIFUNK_GUI_SHOW_WLANSCAN" = "yes" ]; then
cat<<EOF
<INPUT NAME="post_scan" TITLE="Scan nach empfangbaren WLAN-Stationen ausf&uuml;hren" TYPE="SUBMIT" VALUE="WLAN Scan">&nbsp;
EOF
fi

if [ "$FREIFUNK_GUI_SHOW_OLSRINFO" = "yes" ]; then
cat<<EOF
<INPUT NAME="post_olsr" TITLE="OLSR Statusseite anzeigen" TYPE="SUBMIT" VALUE="OLSR-Info">
EOF
fi

cat<<EOF
</TD>
</TR>

<TR><TD COLSPAN="7">&nbsp;</TD>
</TR>
EOF

for DEV in $WLDEV
do
cat<<EOF
<TR>
<TD COLSPAN="7">$DEV</TD>
</TR>
<TR>
<TH>SSID</TH>
<TH>Kanal</TH>
<TH>Ad-Hoc</TH>
<TH>Open</TH>
<TH>Signal</TH>
<TH>Max.</TH>
<TH>BSSID</TH>
</TR>
<TR>
<TD COLSPAN="7">
EOF

#/usr/sbin/wl -i $WLDEV scan
sleep 2
echo '</TR></TR>'
count=0
iwlist $DEV scan | sed \
-e 's/ dBm//g' \
-e 's/)//g' \
-e 's/(//g' \
-e 's/.*Scan completed.*//' \
-e 's/.*Address: /BSSID=/' \
-e 's/.*ESSID:/SSID=/' \
-e 's/.*Mode:/MODE=/' \
-e 's/.*Channel /CHANNEL=/' \
-e 's/.*Signal level=-/RSSI=/' \
-e 's/Noise level=-/; NOISE=/' \
-e 's/.*Encryption key:/CLOSED=/' \
-e 's/.*Mb\/s; /RATE=/' -e 's/Mb\/s//' \
-e 's/.*Extra:.*/END=1/' \
-e 's/.*IE:.*//' \
-e 's/.*Group Cipher.*//' \
-e 's/.*Authentication Suites.*//' \
-e 's/.*Pairwise Ciphers.*//' \
-e 's/.*Preauthentication Supported.*//' \
-e '/^ *$/d' \
| while read line; do
eval $line 2>/dev/null
if [ "$END" == "1" ] ; then
END=""
gif=$(echo $NOISE - $RSSI | bc)
gif=$(echo $gif / 5 | bc)
test $gif -gt 5 && gif=5
test $gif -lt 0 && gif=0
if [ $CLOSED == "on" ] ; then 
OPEN="no"
fi
if [ $CLOSED == "off" ] ; then 
OPEN="yes"
fi
ADHOC="no"
if [ $MODE == "Ad-Hoc" ] ; then 
ADHOC="yes"
fi
cat<<EOF
<TR><TD>$SSID</TD><TD>$CHANNEL</TD>
<TD><IMG SRC="images/$ADHOC.gif" ALT="ad hoc" TITLE="Ad-Hoc mode" ALIGN="ABSMIDDLE"></TD>
<TD><IMG SRC="images/$OPEN.gif" ALT="$OPEN" TITLE="No WEP/WPA" ALIGN="ABSMIDDLE"></TD>
<TD><IMG SRC="images/power$gif.gif" ALT="P=$gif" TITLE="RSSI: $RSSI dBm, Noise: $NOISE dBm" ALIGN="ABSMIDDLE"></TD>
<TD>$RATE</TD><TD>$BSSID</TD></tr>
EOF
fi
done 2>/dev/null
echo '<TR><TD colspan="7">&nbsp;'
done

cat<<EOF
</TD>
</TR>
</TABLE>
</TD>
</TR>
</TABLE>
</TD>
</TR>
</TABLE>
</TD>
</TR>
</TABLE>
</FORM>
EOF

fi
else

cat<<EOF
<H1>Status: &Uuml;bersicht</H1>

<SCRIPT LANGUAGE="JavaScript" TYPE="text/javascript"><!--
function fold(id) {
obj = document.getElementById(id);
obj.style.display = ('none'==obj.style.display?'block':'none');
return false;
}
//--></SCRIPT>

<FORM ACTION="cgi-bin-status.cgi" METHOD="POST"><TABLE CLASS="shadow0" CELLPADDING="0" CELLSPACING="0"><TR><TD><TABLE CLASS="shadow1" CELLPADDING="0" CELLSPACING="0"><TR><TD><TABLE CLASS="shadow2" CELLPADDING="0" CELLSPACING="0"><TR><TD><TABLE BORDER="1"
CELLPADDING="0" CELLSPACING="0" CLASS="formfixwidth"><TR><TD COLSPAN="2">
EOF

cat<<EOF
<INPUT NAME="post_overview" TITLE="Allgemeine Eigenschaften des Ger&auml;tes in einer &Uuml;bersicht anzeigen" TYPE="SUBMIT" VALUE="&Uuml;bersicht">&nbsp;&nbsp;&nbsp;
EOF

if [ "$FREIFUNK_GUI_SHOW_ROUTING" = "yes" ]; then
cat<<EOF
<INPUT NAME="post_route" TITLE="Die aktuelle Routingtabelle des Ger&auml;tes anzeigen" TYPE="SUBMIT" VALUE="Routen">&nbsp;
EOF
fi

if [ "$FREIFUNK_GUI_SHOW_WLANSCAN" = "yes" ]; then
cat<<EOF
<INPUT NAME="post_scan" TITLE="Scan nach empfangbaren WLAN-Stationen ausf&uuml;hren" TYPE="SUBMIT" VALUE="WLAN Scan">&nbsp;
EOF
fi

if [ "$FREIFUNK_GUI_SHOW_OLSRINFO" = "yes" ]; then
cat<<EOF
<INPUT NAME="post_olsr" TITLE="OLSR Statusseite anzeigen" TYPE="SUBMIT" VALUE="OLSR-Info">
EOF
fi

cat<<EOF
</TD>
</TR>
EOF

idx=1
while [ "$idx" -le "$FREIFUNK_INTERFACE_N" ]	
do

# evaluate interface
eval temp='$FREIFUNK_INTERFACE_'$idx'_DEV'
eval temp2='$FREIFUNK_INTERFACE_'$idx'_MASK'
eval temp3='$FREIFUNK_INTERFACE_'$idx'_MAC'
eval temp4='$FREIFUNK_INTERFACE_'$idx'_IP'
# WLMASK=$(ip -f inet addr show dev $DEV label $DEV |sed -ne'2{s# \+inet \([0-9\.\/]\+\).*#\1#;p}')

if [ "$temp3" = '' ]
	then
		{
		$temp3= "IP_NET_x_MAC has to be set for wireless interface in base.txt"
		}
fi

cat<<EOF
<TR>
<TD COLSPAN="2">&nbsp;</TD>
</TR>
<TR>
<TD COLSPAN="2">$temp</TD>
</TR>
<TR>
<TD>IP
Adresse:</TD>
<TD>IP:&nbsp;$temp4,
Maske:&nbsp; $temp2 ,
MAC:&nbsp;$temp3</TD>
</TR>
<TR>
<TD>WLAN-Status:</TD>
<TD>
$(iwconfig $temp 2>&1)</TD>
</TR>
EOF

	# index um eins erhoehen
        idx=`/usr/bin/expr $idx + 1`
done

MYTIME=`/bin/cat /proc/uptime | /usr/bin/cut -d' ' -f1 | /usr/bin/cut -d'.' -f1`
SEC=`/usr/bin/expr $MYTIME % 60`
MYTIME=`/usr/bin/expr $MYTIME / 60`
MIN=`/usr/bin/expr $MYTIME % 60`
MYTIME=`/usr/bin/expr $MYTIME / 60`
HRS=`/usr/bin/expr $MYTIME % 24`
DAYS=`/usr/bin/expr $MYTIME / 24`
LOADMIN=`/bin/cat /proc/loadavg | /usr/bin/cut -d' ' -f3`
LOAD=`/bin/cat /proc/loadavg | /usr/bin/cut -d' ' -f2`
LOADMAX=`/bin/cat /proc/loadavg | /usr/bin/cut -d' ' -f1`

cat<<EOF
<TR>
<TD>Ger&auml;telaufzeit:</TD>
<TD>$HRS:$MIN:$SEC up $DAYS days, $(date "+%H:%M"), load average: $LOADMIN, $LOAD, $LOADMAX</TD>
</TR>
<TR>
<TD>Ger&auml;teinfo:</TD>
<TD>Boardtype: PC compatible</TD>
EOF

if [ "$FREIFUNK_GUI_SHOW_OLSRINFO" = "yes" ]; then

cat<<EOF
</TD>
</TR>
<TR>
<TD>Nachbarn:</TD>
<TD>
EOF

wget -q -O - http://127.0.0.1:2006/neighbours | sed -ne'
/^Table: Links/{
s/.*/<\TABLE FRAME="VOID" BORDER="1" CELLSPACING="0" CELLPADDING="1" WIDTH="100%">/
:n
p
n
s/^[^	]*	//
s/^remote //
s#\([0-9]\+\.[0-9]\+\.[0-9]\+\.[0-9]\+\)\([^/]\)#<A HREF="http://\1/cgi-bin-status.html">\1</A>\2#g
s/	$//
s#	#</TD><TD>#g
s#.\+#<TR><TD STYLE="padding-right:100">&</TD></TR>#
s/./&/
tn
c\
<\/TABLE>
p
}
'
fi

cat<<EOF
</TD>
</TR>
EOF

if [ "$FREIFUNK_GUI_SHOW_BOOTLOG" = "yes" ]; then

cat<<EOF
<TR><TD>Boot-Log: </TD><TD><A HREF="#" ONCLICK="return fold('dmesg')">Ein-/ Ausblenden</A></TD></TR><TR><TD COLSPAN="2">
EOF

echo -n '<PRE STYLE="display:none" ID="dmesg">'
cat /bootmsg.txt 2>&1
echo '</PRE>'
if [ -f /var/log/syslog ] >/dev/null; then

cat<<EOF
</TD>
</TR>


<TR><TD>System-Log:
</TD><TD><A HREF="#" ONCLICK="return fold('logread')">Ein-/ Ausblenden</A></TD>
</TR><TR><TD COLSPAN="2">
EOF

echo -n '<PRE STYLE="display:none" ID="logread">'
cat /var/log/syslog 2>&1
echo '</PRE>'
fi

cat<<EOF
</TD>
</TR>
EOF
fi

if [ "$FREIFUNK_GUI_SHOW_IPNAT" = "yes" ]; then
cat<<EOF
<TR><TD>IP-NAT: </TD><TD><A HREF="#" ONCLICK="return fold('nat')">Ein-/ Ausblenden</A></TD></TR><TR><TD COLSPAN="2">
EOF

echo -n '<PRE STYLE="display:none" ID="nat">'
iptables -t nat -L -n -v 2>&1
echo '</PRE>'

cat<<EOF
</TD>
</TR>
EOF
fi

if [ "$FREIFUNK_GUI_SHOW_INTERFACES" = "yes" ]; then
cat<<EOF
<TR><TD>Schnittstellen-Konfiguration: </TD><TD><A HREF="#" ONCLICK="return fold('ifconfig')">Ein-/ Ausblenden</A></TD></TR><TR><TD COLSPAN="2">
EOF

echo -n '<PRE STYLE="display:none" ID="ifconfig">'

netstat -ie 2>&1

echo '</PRE>'

cat<<EOF
</TD>
</TR>
EOF
fi

if [ "$FREIFUNK_GUI_SHOW_CONNECTIONS" = "yes" ]; then
cat<<EOF
<TR><TD>Aktive Verbindungen: </TD><TD><A HREF="#" ONCLICK="return fold('conntrk')">Ein-/ Ausblenden</A></TD></TR><TR><TD COLSPAN="2">
EOF

echo -n '<PRE STYLE="display:none" ID="conntrk">'
cat /proc/net/ip_conntrack > /tmp/ip_conntrack
awk 'BEGIN{while(0<getline<"/tmp/ip_conntrack"){if(match($0,"src=[0-9\\.]+")){all[substr($0,RSTART+4,RLENGTH-4)]++}}for(i in all){print all[i]"\t"i}}'|sort -n
echo '</PRE>'

cat<<EOF
</TD>
</TR>
EOF
fi

cat<<EOF
</TABLE></TD></TR></TABLE></TD></TR></TABLE></TD></TR></TABLE>

</FORM>
EOF

fi

. ./cgi-bin-post.sh
