#!/bin/sh

#export DATE="4.10.2006";SCRIPT=${0#/rom}
#export TITLE="Dienste"

. ./cgi-bin-pre.sh

cat<<EOF
<H1>Dienste</H1>
<TABLE CLASS="shadow0" CELLPADDING="0" CELLSPACING="0"><TR><TD><TABLE CLASS="shadow1" CELLPADDING="0" CELLSPACING="0"><TR><TD><TABLE CLASS="shadow2" CELLPADDING="0" CELLSPACING="0"><TR><TD><TABLE BORDER="1" CELLPADDING="0" CELLSPACING="0" CLASS="formfixwidth">
<TR>
<TD>Name</TD>
<TD>Source</TD>
<TD>Proto
<TD>Hops
EOF

SED=$(cat /etc/hosts|sed -ne 's/[ 	]*#.*//;/^[0-9]/{s/^\([^ 	]\+\)[ 	]\+\([^ 	]\+\)$/s\/^\1$\/\2\//;p}')
IFS='|'
cat /var/run/services_olsr|sed -ne 's/^#.*//;s/	*#/|/;/./p'|while read l;do
set $l
echo "</tr><tr><td><a href=\"$1\">$3</a></td><td><a href=\"http://$4\">$(echo $4|sed -e "$SED")</a></td><td>$2</td><td>$(ip route ls|grep "^$4 via"|cut -d' ' -f8)</td>"
done
unset IFS

cat<<EOF
</TD>
</TR>
</TABLE></TD></TR></TABLE></TD></TR></TABLE></TD></TR></TABLE>
EOF

. ./cgi-bin-post.sh
