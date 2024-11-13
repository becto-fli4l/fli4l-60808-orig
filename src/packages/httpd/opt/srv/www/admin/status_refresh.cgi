#!/bin/sh
#------------------------------------------------------------------------------
#                                                                  __FLI4LVER__
# /srv/www/admin/status_refresh.cgi
# Creation:     04.10.2005 HH
# Last Update:  $Id$
#------------------------------------------------------------------------------
# get main helper functions
cginame=status
. /srv/www/include/cgi-helper

uptime=`cut -f1 -d"." /proc/uptime`
days=`expr $uptime / 86400`
hours=`expr $uptime % 86400 / 3600 `
minutes=`expr $uptime % 3600 / 60`
#seconds=`expr $uptime % 60`

uptime="$days $_STAT_days, $hours $_STAT_hours $_MN_and $minutes $_STAT_minutes."

ref="parent.document.getElementById(\"uptime\").value=\"$uptime\";"


set -- `imond-send "channels" "pppoe" "cpu" | sed 's/\(OK\|ERR\) //; s/.$//'`
chan="$1"
pppoe="$2"
usage="$3"

ref="$ref parent.document.getElementById(\"cpuusage\").value=\"$usage %\";\
     parent.document.getElementById(\"cpu1\").width=\"$usage\";"

onl=$_STAT_dial
devs=''
for i in `seq 1 $chan`
do
    devs="$devs $i"
done
case $pppoe in
    1) devs="pppoe $devs" ;;
esac

for i in $devs
do
    cname=
    cdir=
    cip=
    conltime=
    cirate=
    corate=
    cibytes=
    cobytes=
    ctime=
    chtime=
    charge=
    conninfo=`imond-send "status $i" "phone $i" "inout $i" "ip $i" "online-time $i" "quantity $i" | sed 's/OK //; s/.$//'`
    set -- $conninfo
    case $1 in
        Off*)
            cimg=rotaus
        ;;
        Call*)
            cimg=gelbaus
            cname=$2
        ;;
        On*)
            cimg=gruenaus
            cname=$2
            eval cdir='$_STAT_'$3
            case $# in 9) cip=$4; shift 1 ;; esac
            conltime=$4
            cibytes=`bytes2read $5 $6`B
            cobytes=`bytes2read $7 $8`B
            onl=$_STAT_hangup
        ;;
    esac

    ref="$ref parent.document.getElementById(\"cimg${i}\").src=\"../img/$cimg.gif\";\
         parent.document.getElementById(\"cname${i}\").value=\"$cname\";\
         parent.document.getElementById(\"cdir${i}\").value=\"$cdir\";\
         parent.document.getElementById(\"cip${i}\").value=\"$cip\";\
         parent.document.getElementById(\"cibytes${i}\").value=\"$cibytes\";\
         parent.document.getElementById(\"cobytes${i}\").value=\"$cobytes\";\
         parent.document.getElementById(\"conltime${i}\").value=\"$conltime\";\
         parent.document.getElementById(\"dial\").value=\"$onl\";\
         "
done
show_html_header "hidden-status-refresh" "refresh=5;url=$myname;showmenu=no;"

echo '
<script language="JavaScript" type="text/javascript">
parent.document.getElementById("refresh").style.display="block";'
echo '
function hideit()
{
    parent.document.getElementById("refresh").style.display="none";
} '
echo "$ref"
echo 'setTimeout("hideit()",1000);'
echo '</script>'

show_html_footer
