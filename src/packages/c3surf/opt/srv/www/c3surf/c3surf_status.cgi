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
# first of all
# parse variables
eval "`proccgi $*`"
# variables: $FORM_<name>
nstatus=0
myrefresh=60
# c3surf global vars
. /var/run/c3surf.conf
# language
# Thx to Wieland Große for helping - from Mx. ;-)
. /srv/www/c3surf/lang/default
. /srv/www/c3surf/lang/c3surf.$c3surf_login_lang
# code starts here
if [ "$FORM_action" = "logout" ]
then
  /usr/local/bin/c3surf_worker.sh "doLogout" "$REMOTE_ADDR"
  nstatus=7
#  isstatus="Logout done."
  isstatus="$_c3surf_status_logoutexit"
  myrefresh=5
#  echo "Location: /c3surf_status.cgi"
#  echo
#  exit
fi

####
# Status bestimmen
####
if [ "$nstatus" = "0" ]
then
  # test status
  if test -s $C3SURF_TMP_PATH/$REMOTE_ADDR.status
  then
    isstatus=$(eval echo "$`cat $C3SURF_TMP_PATH/$REMOTE_ADDR.status`")
    nstatus=7
    rm -f $C3SURF_TMP_PATH/$REMOTE_ADDR.status
    myrefresh=5
  elif test -s $C3SURF_TMP_PATH/$REMOTE_ADDR.time
  then
    {
      read c3surf_time fscount fsid fsip fsmac fsvorname fsname fsmail
    } < $C3SURF_TMP_PATH/$REMOTE_ADDR.time
    nstatus=1
    if [ $c3surf_time -eq 0 ]
    then
      # unlimited user account
#        isstatus="unlimited"
        isstatus="$_c3surf_status_unlimited"
    else
      c3surf_now=`date +%s`
      c3surf_time=`/usr/bin/expr $c3surf_time - $c3surf_now`
      c3surf_time=`/usr/bin/expr $c3surf_time / 60`
      if [ "$c3surf_time" -le "1" ]
      then
#        isstatus="you still have some seconds!"
        isstatus="$_c3surf_status_seconds"
      else
#        isstatus="you have $c3surf_time minutes"
        isstatus="$_c3surf_status_minutes_1 $c3surf_time $_c3surf_status_minutes_2"
      fi
    fi # unlimited
  elif test -f $C3SURF_SYSLOCK_FILE
  then
    isstatus=$(eval echo "$`cat $C3SURF_SYSLOCK_FILE`")
    nstatus=9
  else
#    isstatus="You are not logged in."
    isstatus="$_c3surf_status_loggedout"
    nstatus=2
  fi
fi
# ####
# html
# ####
# #############################################################################
# Kaspersky glaubt an phishing
#cat <<EOF
#    Content-Type: text/html; charset=utf-8
#    Cache-Control: max-age=1, s-maxage=1, no-cache, must-revalidate, no-store
#    Pragma: no-cache
#EOF
#echo "Expires: `date -Ru`"
#echo
# #############################################################################

# thx to Manfred: tell doctype with echo - cat forces Kapersky to alert phishing
echo "Content-Type: text/html; charset=utf-8"
echo "Cache-Control: max-age=1, s-maxage=1, no-cache, must-revalidate, no-store"
echo "Pragma: no-cache"
echo "Expires: `date -Ru`"
echo

# we do not need any longer this tags in the <head>
#  <meta http-equiv="Cache-Control:" content="max-age=1, s-maxage=1, no-cache, must-revalidate, no-stor">
#  <meta http-equiv="Pragma" content="NO-CACHE">

cat <<EOF
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta http-equiv="expires" content="0">
<meta HTTP-EQUIV="Refresh" CONTENT="$myrefresh;url=/c3surf_status.cgi">
<meta name="description" content="c3surf online status">
<meta name="author" content="Frank Saurbier">
<meta name="robots" content="noindex">
<title>FreeSurf and LoginUsr Status</title>
</head>
<body text="#000000" bgcolor="#E0E0E0" link="#0000FF" vlink="#800000" alink="#FF0000">
<center>
<form action="/c3surf_status.cgi" method="get" enctype="text/plain">
<table border=0>
<colgroup>
<col width="125">
<col width="550">
<col width="125">
</colgroup>
<tr>
<td align="center">
<font color="#408080" size=+1>
$_c3surf_status_status
</font>
</td>
<td align=center>
<font size=+1>
EOF
# ###############
# Status ausgeben
# ###############
if [ "$OPT_LOGINUSR" = "yes" ]
then
  if [ -n "$fsid" ]
  then
    if [ "$fsid" = "-" ]
    then
      echo "<font color=\"#CC8000\" size=+1>$_c3surf_status_host_unlimited</font>"
    else
      echo "<font color=\"#408080\" size=+1>$fsid</font>: $isstatus"
    fi
  else
    echo "$isstatus"
  fi
else # --- FreeSurf ---
  if [ "$fsid" = "-" ]
  then
    echo "<font color=\"#CC8000\" size=+1>$_c3surf_status_host_unlimited</font>"
  else
    echo "$isstatus"
  fi
fi
#
cat <<EOF
</font>
<input type="hidden" name="action" value="logout">
</td>
<td align=center>
EOF
#
## jetzt noch den Abmeldeknopf zeigen oder nicht
#
case $nstatus in
  1) echo "<input type=\"submit\" value=\"$_c3surf_status_logout\">" ;;
  2) echo "$_c3surf_status_login" ;;
  7) echo "$_c3surf_status_info" ;;
  8) echo "$_c3surf_status_error" ;;
  9) echo "$_c3surf_status_block" ;;
  *) echo "$_c3surf_status_there4" ;;
esac
# Ende
cat <<EOF
</td>
</tr>
</table></center>
</form>
</body>
</html>
