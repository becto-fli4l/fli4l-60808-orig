#!/bin/sh
##------------------------------------------------------------------------------
## c3Surf - login for services                                      __FLI4LVER__
##
## Creation:    07.01.2008 Frank Saurbier - c3Surf@arcor.de
## Last Update: $Id$
##
## Copyright (c) 2008-2010 - Frank Saurbier <c3surf@arcor.de>
## Copyright (c) 2010-2016 - Frank Saurbier, fli4l-Team <team@fli4l.de>
##
## Licence and conditions look at ~/config/c3surf.txt
##-------------------------------------------------------------------------------

. /srv/www/include/cgi-helper
. /var/run/c3surf.conf
if [ -n $FORM_language ]
then
if [ -f /srv/www/lang/c3surf.$FORM_language ]
then
. /srv/www/lang/c3surf.$FORM_language
fi
fi
my_n=0
my_s=0
simple_output ()
{
htmlspecialchars | while read line
do
fsuid=`echo $line | cut -d "," -f 1`
fspwd=`echo $line | cut -d "," -f 2`
fstime=`echo $line | cut -d "," -f 3`
fsvalid=`echo $line | cut -d "," -f 4`
fslives=`echo $line | cut -d "," -f 5`
my_foot="$_prt_h_foot_1"
if [ -n "$fslives" ]
then
if [ $fslives -lt 0 ]; then my_foot="$_prt_h_foot_3"; fi
if [ $fslives -gt 0 ]; then my_foot="$_prt_h_foot_2a $fslives $_prt_h_foot_2b"; fi
fi
if [ $my_n -ne 0 ]
then
echo "<br><br>"
if [ $my_s -ge 5 ]
then
echo "<br style="page-break-before:always">"
my_s=0
fi

fi
my_s=`/usr/bin/expr $my_s "+" 1`
my_n=`/usr/bin/expr $my_n "+" 1`
[ $my_n -ge 5 ] && $my_n=0
cat <<-EOF
<center><table border=2>
<colgroup>
  <col width="200">
  <col width="130">
  <col width="90">
  <col width="250">
</colgroup>
<tr>
<th colspan="4"><center><br>$_prt_h_head<br>&nbsp;</center></th>
</tr>
EOF
cat <<-EOF
<tr>
<th><center>$_prt_h_uid</center></th>
<th><center>$_prt_h_pwd</center></th>
<th><center>$_prt_h_time</center></th>
<th><center>$_prt_h_valid</center></th>
</tr>
EOF
cat <<-EOF
<tr>
<td><center><nobr>$fsuid</nobr></center></td>
<td><center><nobr>$fspwd</nobr></center></td>
<td><center><nobr>$fstime</nobr></center></td>
<td><center><nobr>$fsvalid</nobr></center></td>
</tr>
EOF
cat <<-EOF
<tr>
<td colspan="4"><center>$my_foot</center></td>
</tr>
</table>

EOF
done
}
echo "Content-Type: text/html; charset=utf-8"
echo "Cache-Control: max-age=1, s-maxage=1, no-cache, must-revalidate, no-store"
echo "Pragma: no-cache"
echo "Expires: `date -Ru`"
echo
cat <<-EOF
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta http-equiv="expires" content="0">
<meta name="description" content="Voucher Printer">
<meta name="author" content="Frank Saurbier">
<meta name="robots" content="noindex">
<title>Print Voucher</title>
<style type="text/css">
@page { size:21.0cm 29.7cm;
        margin-top:1.5cm;
        margin-bottom:1.5cm;
        margin-left:1.5cm;
        margin-right:1.5cm
}
</style>
</head>
<body text="#000000" bgcolor="#F0F0F0" link="#0000FF" vlink="#800000" alink="#FF0000">
EOF
[ -f $C3SURF_VOUCHER_PRINTFILE ] && /bin/cat $C3SURF_VOUCHER_PRINTFILE | simple_output
cat <<-EOF
<!--html footer-->
</body>
</html>
