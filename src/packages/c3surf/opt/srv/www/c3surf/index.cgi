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
#
# ###########################################################################
# eigenes interface, nicht fli4l Admin - macht alles noch ohne cgi-helper
# verwendet jetzt method "POST" 
# kann aber auch mit "GET" genutzt werden (nur FreeSurf, nicht bei LOGINUSR!)
# ###########################################################################

# Package Vars
. /var/run/c3surf.conf

# POST Variablen holen, nur ohne LOGINUSR Option
if [ "$OPT_LOGINUSR" != "yes" ]
then
  eval "`proccgi $*`"
  mytitle="Free Surf"
else
  mytitle="Login User"
fi

# funzt nicht: sollte Portal-Funktion werden: muss noch forschen
# Infos an den Benutzer vor der Anmeldung geben?
# if [ "$LOGINUSR_SHOW_INFO" = "no" ]
# then
#   echo "Location: http://$C3SURF_HTTPD_HOST_NAME/login/index.cgi?action=login&time=`date +%s`"
#   echo
#   exit
# fi

# Vorsicht mit z.B. backticks, also umwandeln und nur Zeichen 'A-Za-z0-9@‰¸ˆƒ‹÷ﬂ.' zulassen
if [ "$OPT_LOGINUSR" = "yes" ]
then
  # lese Daten der Anmeldung
  if [ -f "$C3SURF_TMP_PATH/$REMOTE_ADDR.time" ]
  then
    {
      read fstime fscount fsid fsip fsmac fsvorname fsname fsmail
    } < $C3SURF_TMP_PATH/$REMOTE_ADDR.time
  fi
  # etwas aufr‰umen
  unset fstime fscount fsmac
else
  if [ "$REQUEST_METHOD" = "GET" ]
  then
    # Verarbeitung FORM_vorname, -name und mail
    # eigentlich obsolete, da method "POST" gew‰hlt ist - Aufruf ist aber mˆglich
    # index.cgi?action=unlock&fsvorname=<vorname>&fsname=<name>&fsmail=<mailadresse>
    if [ "$FORM_action" = "unlock" ]
    then
      fsvorname=$(echo "$FORM_fsvorname" | tr -d '\n' | tr -c '‰¸ˆƒ‹÷ﬂ\-[:alnum:]' '_')
      fsname=$(echo "$FORM_fsname" | tr -d '\n' | tr -c '‰¸ˆƒ‹÷ﬂ\-[:alnum:]' '_')
      fsmail=$(echo "$FORM_fsmail" | tr -d '\n' | tr -c '@‰¸ˆƒ‹÷ﬂ\-.[:alnum:]' '_')
      fsid=$$
    fi
  else # POST setzt automatisch die fsid
    eval $(echo "$FORM_action" | tr -d '\r' | tr '\n' ';' | tr -c '@‰¸ˆƒ‹÷ﬂ\-.=;[:alnum:]' '_')
  fi
fi

# page logging
if [ "$C3SURF_DOLOG_PAGE" = "yes" ]
then
  /usr/local/bin/c3surf_log_page.sh "$C3SURF_LOG_PATH" "$REMOTE_ADDR" "$HTTP_USER_AGENT" "$fsid" "$fsvorname" "$fsname" "$fsmail"
fi

# Action: login
if [ "$REQUEST_METHOD" = "GET" ]
then
  if [ "$FORM_action" = "unlock" ]
  then
    /usr/local/bin/c3surf_worker.sh "doLogin" "$REMOTE_ADDR" "$fsid" "$fsvorname" "$fsname" "$fsmail"
  fi
else
  # das war ein POST also action bitte
  if [ "$OPT_LOGINUSR" = "yes" ]
  then
    echo "Location: http://$C3SURF_HTTPD_HOST_NAME/login/index.cgi?action=login&time=`date +%s`"
    echo
    exit 0
  else
    /usr/local/bin/c3surf_worker.sh "doLogin" "$REMOTE_ADDR" "$fsid" "$fsvorname" "$fsname" "$fsmail"
  fi
fi

# checke erstmal ob wir Umleiten m¸ssen
# ###
# passe an den richtigen Host an
# ###
if [ "$REQUEST_METHOD" = "POST" ]
then
  mynewurl="<meta HTTP-EQUIV=\"Refresh\" CONTENT=\"3;url=http://$C3SURF_HTTPD_HOST_NAME/\">"
else
  # rc-script now translates capital letters in C3SURF_HTTPD_HOST_NAME
  # HTTP_HOST has no capital letters, but we do a translate - it could change in future
  if [ "$(echo "$HTTP_HOST" | tr 'A-Z' 'a-z')" != "$C3SURF_HTTPD_HOST_NAME" ]
  then
    # IE reports: Umleitungsfehler (war gross-klein schreib-Problem)
    echo "Location: http://$C3SURF_HTTPD_HOST_NAME/"
    echo
    exit 0
    # Manfred: flackernde Variante mit 0 ==> besser mit 3
    # flackern war ebenfalls durch gross-klein-schreib-Problem entstanden, daher wieder 0.
    # the coding above is the better solution.
    # mynewurl="<meta HTTP-EQUIV=\"Refresh\" CONTENT=\"0;url=http://$C3SURF_HTTPD_HOST_NAME/\">"
  else
    if [ "$FORM_action" = "unlock" ]
    then
      mynewurl="<meta HTTP-EQUIV=\"Refresh\" CONTENT=\"3;url=http://$C3SURF_HTTPD_HOST_NAME/\">"
    else
      mynewurl="<!-- keine Umleitung -->"
    fi
  fi
fi
# mynewurl enth‰lt den Rest des html head

# language, do this work late to save resources
# Thx to Wieland Groﬂe for helping - from Mx. ;-)
. /srv/www/c3surf/lang/default
if [ ! -f /srv/www/c3surf/lang/c3surf.$c3surf_login_lang ]
then
  # we use english language if the selected language file does not exist
  c3surf_login_lang="en"
  echo "c3surf_login_lang=\"en\"" > /srv/www/c3surf/lang/default 
fi
# this is correct now
. /srv/www/c3surf/lang/c3surf.$c3surf_login_lang
select other languages
myLangSelect=""
for f in /srv/www/c3surf/lang/c3surf.*
do
  # schlieﬂe den Abfrage-String und die selektierte Sprache aus
  if [ "$f" != "/srv/www/c3surf/lang/c3surf.*" -a "$f" != "/srv/www/c3surf/lang/c3surf.$c3surf_login_lang" ]
  then
    otherLang=${f#*.}
    myLangSelect="$myLangSelect&nbsp;&lsaquo;<a href=\"c3surf_setlang.cgi?language=$otherLang\">$otherLang</a>&rsaquo;"
  fi
done
if [ -n "$myLangSelect" ]
then
  myLangSelect="&nbsp;&mdash;$myLangSelect"
fi

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
$mynewurl
<meta name="description" content="c3surf portal">
<meta name="author" content="Frank Saurbier">
<meta name="robots" content="noindex">
<title>$mytitle</title>
</head>
<body background="/pic/backfs03.jpg" text="#000000" bgcolor="#F0F0F0" link="#0000FF" vlink="#800000" alink="#FF0000">
<div align="center"><font size="+2">$_c3surf_userportal_title_1<font color="#408080">$_c3surf_userportal_title_2</font></font></div>
<div align="center"><font size="+1" color="#408080">$_c3surf_userportal_language</font>$myLangSelect</div>
<hr noshade size="1">
<center><table border=0>
<colgroup>
<col width="75">
<col width="50">
<col width="600">
<col width="75">
</colgroup>
<tr>
<!--Inhalt einer Zelle, die sich ¸ber 4 Zeilen erstreckt //-->
<td valign="top" rowspan="4">
</td>
<td valign="middle" align="center">
</td>
<td>
<font color="#408080" size=+2>$_c3surf_userportal_willkommen_1&nbsp;</font>$_c3surf_userportal_willkommen_2<br>
</td>
<td valign="bottom" rowspan="4">
<!-- delete this line: <img src="/pic/wormpeeking.gif" border="0" alt=""> -->
</td>
</tr>
<!--2.Zeile-->
<tr>
<td valign="middle" align="center">
</td>
<td>
$_c3surf_userportal_text_title
</td>
</tr>
<!--3.Zeile-->
<tr>
<td valign="middle" align="center">
</td>
<td>
<font color="#408080">
<li>$_c3surf_userportal_text_1</li>
<li>$_c3surf_userportal_text_2</li>
<li>$_c3surf_userportal_text_3</li>
<li>$_c3surf_userportal_text_4</li>
<li>$_c3surf_userportal_text_5</li>

</font>
</td>
</tr>
<!--4.Zeile-->
<tr>
<td valign="middle" align="center">
</td>
<td>$_c3surf_userportal_text_6</td>
</tr>
</table></center>
<hr noshade size="1">
<!--Formular-->
<center>
<form action="/index.cgi" method="post" enctype="text/plain">
<input type="hidden" name="action" value="fsid=$$">
<table border=0>
<colgroup>
<col width="125">
<col width="550">
<col width="125">
</colgroup>
<!--1.Zeile-->
<tr>
<td align="center" rowspan="3">
<font color="#408080" size=+1>
$_c3surf_userportal_anmeldezeile_1
<br>
$_c3surf_userportal_anmeldezeile_2</font>
</td>
EOF
# ###
if [ "$OPT_LOGINUSR" = "yes" ]
then
# ###
cat <<EOF
<td rowspan="3" align=center>
$_c3surf_userportal_anmeldezeile_3<br>
</td>
EOF
# ###
# echo "<font color=\"#408080\">Bitte zuerst abmelden, falls die Statusanzeige einen anderen Benutzer zeigt.</font>"
else
# ###
cat <<EOF
<td rowspan="3" align=center>
<table border=0>
<tr>
<td align=right>$_c3surf_userportal_form_1:</td>
<td><tt><input type=text size=50 name="fsvorname" maxlength="50"></tt></td>
</tr>
<tr>
<td align=right>$_c3surf_userportal_form_2:</td>
<td><tt><input type=text size=50 name="fsname" maxlength="50"></tt></td>
</tr>
<tr>
<td align=right>$_c3surf_userportal_form_3:</td>
<td><tt><input type=text size=50 name="fsmail" maxlength="50"></tt></td>
</tr>
</table>
</td>
EOF
# ###
fi
# ###

cat <<EOF
<!--1.Zeile Haupttabelle-->
<td rowspan="3" align=center>
<input type=submit value="$_c3surf_userportal_loginbutton">
</form>
</td>
</tr>
</table></center>
<hr noshade size="1">
<!--Status-->
<iframe src="/c3surf_status.cgi" width="100%" height="70" align="center" name="Status">
<p>$_c3surf_userportal_iframes
<a href="/c3surf_status.cgi">$_c3surf_userportal_iframes_link</a></p>
</iframe>
<!--html footer-->
<hr noshade size="1">
<div align="center"><nobr>
$_c3surf_userportal_form_footer 14.08.2009 (fs)
</nobr></div>
</body>
</html>
