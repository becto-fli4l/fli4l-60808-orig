#!/bin/sh
#---------------------------------------------------------------------
# /srv/www/admin/yapsGUI_send.cgi                         __FLI4LVER__
#
# Last Update:  $Id$
#
# report errors to: Christoph Schulz <fli4l@kristov.de>
#---------------------------------------------------------------------

SEC_REALM='sms'
SEC_ACTION='send'
. ./sec-lib.cgi

# variablen einlesen
eval "`proccgi $*`"

echo "Content-Type: text/html; charset=utf-8"
echo "Pragma: no-cache"
echo "Expires: 0"
echo ""

#if [ -d $YAPSGUI_LOGPATH ]
# then
# else
#  mkdir $YAPSGUI_LOGPATH
#fi

if [ -n "$FORM_RecipientM" ]
then
   FORM_Recipient=$FORM_RecipientM
fi

if [ -n "$FORM_SenderM" ]
then
  FORM_Sender=$FORM_SenderM
fi

if [ -z "$FORM_Recipient" ]
then
  ErrorMessage="$ErrorMessage <li>Kein Empf&auml;nger angegeben"
fi

if [ -z "$FORM_Sender" ]
then
  ErrorMessage="$ErrorMessage <li>Kein Absender angegeben"
fi

if [ -z "$FORM_Message" ]
then
   ErrorMessage="$ErrorMessage <li>Keine Nachricht angegeben"
fi

if [ -n "$ErrorMessage" ]
then
  echo "<html><body><h2><font color=red>Es sind Fehler aufgetreten:</font></h2><ul>$ErrorMessage</ul></body></html>"
  exit
fi


echo "
<html>
<head>
<title>yaps-GUI</title>
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">
<meta name=\"author\" content=\"Das fli4l-Team, team@fli4l.de\">
<LINK REL=STYLESHEET TYPE=\"text/css\" HREF=\"main.css\">
</head>

<body bgcolor=\"#FFFFFF\" text=\"#000000\">
<h3 align=\"center\"><font face=\"Verdana, Arial, Helvetica, sans-serif\"><div id=sendtext>Bitte warten...
   Nachricht wird versandt...</div></font></h3>
<table width=\"80%\" border=\"1\" cellspacing=\"2\" cellpadding=\"2\" align=\"center\" bordercolor=\"#000000\">
  <tr> 
    <td colspan=\"2\">
      <h3 align=\"center\"><font face=\"Verdana, Arial, Helvetica, sans-serif\">Nachricht an yaps &uuml;bergeben - Ergebnis:</font></h3>
    </td>
  </tr>
  <tr valign=\"top\"> 
    <td width=\"50%\"><font face=\"Verdana, Arial, Helvetica, sans-serif\">Absender</font></td>
    <td width=\"50%\"><font face=\"Verdana, Arial, Helvetica, sans-serif\">"
# Absender
echo "$FORM_Sender"
echo "</font></td>
  </tr>
  <tr valign=\"top\"> 
    <td width=\"50%\"><font face=\"Verdana, Arial, Helvetica, sans-serif\">Empf&auml;nger</font></td>
    <td width=\"50%\"><font face=\"Verdana, Arial, Helvetica, sans-serif\">"
# Empf�nger
echo "$FORM_Recipient"
echo "</font></td>
  </tr>
  <tr valign=\"top\"> 
    <td width=\"50%\"><font face=\"Verdana, Arial, Helvetica, sans-serif\">Nachricht</font></td>
    <td width=\"50%\"><font face=\"Verdana, Arial, Helvetica, sans-serif\">"
# Nachricht
echo "$FORM_Message"
echo "</font></td>
  </tr>
  <tr valign=\"top\"> 
    <td width=\"50%\"> 
      <font face=\"Verdana, Arial, Helvetica, sans-serif\">Status</font>
    </td>
    <td width=\"50%\"> 
      <pre><font face=\"Verdana, Arial, Helvetica, sans-serif\" size=\"-1\"><b>"
# Status
if [ "$FORM_Debug" = "1" ]
then
 DebugLevel="-v"
fi
if [ "$FORM_Debug" = "2" ]
then
 DebugLevel="-vv"
fi
if [ "$FORM_Debug" = "3" ]
then
 DebugLevel="-vvv"
fi
if [ "$FORM_Debug" = "4" ]
then
 DebugLevel="-vvvv"
fi
if [ "$FORM_RequestConfirm" = "1" ]
then
   ReqCon="-r"
fi

Datum=`date +"%d%m%y%H%M"`
if [ -z "$FORM_Sender" ]
then
# echo "yaps -v $FORM_Recipient $FORM_Message"
yaps $DebugLevel $ReqCon $FORM_Recipient "$FORM_Message" # -l $YAPSGUI_LOGPATH$Datum.$FORM_Sender.$FORM_Recipient.$FORM_Message
 else
# echo "yaps -c $FORM_Sender -v $FORM_Recipient $FORM_Message "
yaps -c $FORM_Sender $ReqCon $DebugLevel $FORM_Recipient "$FORM_Message" # -l $YAPSGUI_LOGPATH$Datum.$FORM_Sender.$FORM_Recipient.$FORM_Message
fi
echo "</b></font></pre>
    </td>
  </tr>
</table>
<script language=JavaScript>
  document.all.sendtext.innerText=\"\";
</script>
</body>
</html>"

