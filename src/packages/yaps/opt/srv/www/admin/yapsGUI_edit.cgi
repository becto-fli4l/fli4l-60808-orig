#!/bin/sh
#---------------------------------------------------------------------
# /rv/ww/admin/yapsGUI_edit.cgi                           __FLI4LVER__
#
# Last Update:  $Id$
#
# report errors to: Christoph Schulz <fli4l@kristov.de>
#---------------------------------------------------------------------

SEC_REALM='sms'
SEC_ACTION='edittb'
. ./sec-lib.cgi
. /var/run/yapsgui.conf

echo "Content-Type: text/html; charset=utf-8"
echo "Pragma: no-cache"
echo "Expires: 0"
echo ""

idx=1
while [ "$idx" -lt "$YAPSGUI_USER_N" ]
do
  eval username='$YAPSGUI_USER_'$idx'_USERNAME'
  if [ "$username" = "$REMOTE_USER" ]
  then
    eval YAPSGUI_RECIPIENT_TB='$YAPSGUI_USER_'$idx'_RECIPIENT_TB'
    # eval YAPSGUI_RECIPIENT_STD='$YAPSGUI_USER_'$idx'_RECIPIENT_STD'
    eval YAPSGUI_SENDER_TB='$YAPSGUI_USER_'$idx'_SENDER_TB'
    # eval YAPSGUI_SENDER_STD='$YAPSGUI_USER_'$idx'_SENDER_STD'
    YAPSGUI_USER="$username"
  fi
  idx=`/usr/bin/expr $idx + 1`
done

if [ -z "$YAPSGUI_USER" ]
then
  YAPSGUI_RECIPIENT_TB=$YAPSGUI_RECIPIENT_TB_COMMON
  # YAPSGUI_RECIPIENT_STD=$YAPSGUI_RECIPIENT_STD_COMMON
  YAPSGUI_SENDER_TB=$YAPSGUI_SENDER_TB_COMMON
  # YAPSGUI_SENDER_STD=$YAPSGUI_SENDER_STD_COMMON
fi

if [ ! -f "$YAPSGUI_RECIPIENT_TB" ]
then
  if [ ! -d `dirname "$YAPSGUI_RECIPIENT_TB"` ]
  then
    mkdir -p `dirname "$YAPSGUI_RECIPIENT_TB"`
  fi
  echo > "$YAPSGUI_RECIPIENT_TB"
fi

if [ ! -f "$YAPSGUI_SENDER_TB" ]
then
  if [ ! -d `dirname "$YAPSGUI_SENDER_TB"` ]
  then
    mkdir -p `dirname "$YAPSGUI_SENDER_TB"`
  fi
  echo > "$YAPSGUI_SENDER_TB"
fi

dtou -s "$YAPSGUI_SENDER_TB"
dtou -s "$YAPSGUI_RECIPIENT_TB"

# variablen einlesen
eval "`proccgi $*`"

if [ "$FORM_action" = "edit" ]
then
    if [ "$FORM_tb" = "sender" ]
    then
        phonebook="$YAPSGUI_SENDER_TB"
    else
        phonebook="$YAPSGUI_RECIPIENT_TB"
    fi
    
    # print header
    echo "
    <html>
    <head>
    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">
    <meta name=\"author\" content=\"Das fli4l-Team, team@fli4l.de\">
    <title>yaps-GUI 2</title>
    <LINK REL=STYLESHEET TYPE=\"text/css\" HREF=\"main.css\">
    </head>
    <body bgcolor=\"#FFFFFF\" text=\"#000000\">
    <nobr>
    <h2 align=\"center\"><font color=\"#D5D5D5\">_</font><font color=\"#AAAAAA\">_</font><font color=\"#808080\">_</font><font color=\"#555555\">_</font><font color=\"#2B2B2B\">_</font><font face=\"Verdana, Arial, Helvetica, sans-serif\">Telefonbuch
      bearbeiten</font><font color=\"#2B2B2B\">_</font><font color=\"#555555\">_</font><font color=\"#808080\">_</font><font color=\"#AAAAAA\">_</font><font color=\"#D5D5D5\">_</font></h2></nobr>
      <font size=-1>Beispiel:
      <blockquote>
      0162123123=Meine Tante<br>
      0171666666=Graf Wolfgang Meier-Zeppelin
      </blockquote></font>
      <center>
      <form name=\"frmSend\" method=\"post\" action=\"yapsGUI_edit.cgi\">
        <p>
          <input type=\"hidden\" name=\"action\" value=\"commit\">
          <input type=\"hidden\" name=\"tb\" value=\"$FORM_tb\">
          <textarea name=\"file\" cols=\"60\" rows=\"30\">"
#    while read line
#    do
#        echo $line
#    done < $phonebook
    echo `cat $phonebook|tr '\n' '\r'`
    echo "</textarea>
        </p>
        <p><font face=\"Verdana, Arial, Helvetica, sans-serif\">
          <input type=\"reset\" name=\"Reset2\" value=\"Reset\">
          <input type=\"submit\" value=\"Datei schreiben\" name=\"submit\">
          </font></p>
    </form>
    </center>
    </body></html>"  
elif [ "$FORM_action" = "commit" ]
then
    if [ "$FORM_tb" = "sender" ]
    then
        phonebook="$YAPSGUI_SENDER_TB"
    else
        phonebook="$YAPSGUI_RECIPIENT_TB"
    fi
    
    echo "$FORM_file" > $phonebook
    dtou -s $phonebook
    
    echo "<html><body>"
    echo "<h1>&Auml;nderungen &uuml;bernommen</h1>"
    echo "<center>"
    echo "<a href=main_yapsGUI.cgi>zur&uuml;ck</a>"  
    echo "</center>"
    echo "</body></html>"
else
    echo "<html><body>"
    echo "<h1>FATAL ERROR</h1>"
    echo "<font color=red><center>"
    echo "Unknown Command Line"  
    echo "</center></font>"
    echo "</body></html>"
fi

