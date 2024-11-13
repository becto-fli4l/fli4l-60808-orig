#!/bin/sh
#---------------------------------------------------------------------
# /srv/www/admin/main_yapsGUI.cgi                         __FLI4LVER__
#
# Last Update:  $Id$
#
# report errors to: Christoph Schulz <fli4l@kristov.de>
#---------------------------------------------------------------------

SEC_REALM='sms'
SEC_ACTION='send'
. ./sec-lib.cgi
. /var/run/yapsgui.conf

echo "Content-Type: text/html; charset=utf-8"
echo "Pragma: no-cache"
echo "Expires: 0"
echo ""

for idx in `seq 1 "$YAPSGUI_USER_N"`
do
  eval username='$YAPSGUI_USER_'$idx'_USERNAME'
  if [ "$username" = "$REMOTE_USER" ]
  then
    eval YAPSGUI_RECIPIENT_TB='$YAPSGUI_USER_'$idx'_RECIPIENT_TB'
    eval YAPSGUI_RECIPIENT_STD='$YAPSGUI_USER_'$idx'_RECIPIENT_STD'
    eval YAPSGUI_SENDER_TB='$YAPSGUI_USER_'$idx'_SENDER_TB'
    eval YAPSGUI_SENDER_STD='$YAPSGUI_USER_'$idx'_SENDER_STD'
    YAPSGUI_USER="$username"
  fi
done

if [ -z "$YAPSGUI_USER" ]
then
  YAPSGUI_RECIPIENT_TB=$YAPSGUI_RECIPIENT_TB_COMMON
  YAPSGUI_RECIPIENT_STD=$YAPSGUI_RECIPIENT_STD_COMMON
  YAPSGUI_SENDER_TB=$YAPSGUI_SENDER_TB_COMMON
  YAPSGUI_SENDER_STD=$YAPSGUI_SENDER_STD_COMMON
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

# print header
echo "
<html>
<head>
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">
<meta name=\"author\" content=\"Das fli4l-Team, team@fli4l.de\">
<title>yaps-GUI 3</title>
<LINK REL=STYLESHEET TYPE=\"text/css\" HREF=main.css>"


echo "<script language=\"JavaScript\">
function CheckKey(){
        var ie = document.all ? 1 : 0;
        if (ie){
        if (event.keyCode == 13) {
                return false;
        }}
        Target = document.frmSend.Message
        StrLen = Target.value.length
        if (StrLen == 1 && Target.value.substring(0,1) == ' ') {
                Target.value = ''
                StrLen = 0 
        }
        if (StrLen > 160) {
                Target.value = Target.value.substring(0,160)
                CharsLeft = 0 
        }else{
                CharsLeft = 160 - StrLen 
        }
        document.frmSend.Remaining.value = CharsLeft
        // delete und backspace immer erlauben
        if (ie){
        if (event.keyCode == 46||event.keyCode == 8){
                return true;
        }}
        if (CharsLeft == 0) {
                return false;
        }else{
                return true;
        }
}
// setTimeout(\"CheckKey()\", 1000);
</script>"

echo "</head>
<body bgcolor=\"#FFFFFF\" text=\"#000000\">
<nobr>
<h2 align=\"center\"><font color=\"#D5D5D5\">_</font><font color=\"#AAAAAA\">_</font><font color=\"#808080\">_</font><font color=\"#555555\">_</font><font color=\"#2B2B2B\">_</font><font face=\"Verdana, Arial, Helvetica, sans-serif\">yaps-Benutzerschnittstelle 
  v.2</font><font color=\"#2B2B2B\">_</font><font color=\"#555555\">_</font><font color=\"#808080\">_</font><font color=\"#AAAAAA\">_</font><font color=\"#D5D5D5\">_</font></h1>
</nobr>
<form name=\"frmSend\" method=\"post\" action=yapsGUI_send.cgi>
  <table width=\"100%\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\">
    <tr valign=\"top\"> 
      <td width=\"1%\" align=\"center\"> 
        <p><font face=\"Verdana, Arial, Helvetica, sans-serif\"><b>Absender<br>
          </b><font size=\"1\">[<a href=yapsGUI_edit.cgi?action=edit&tb=sender>TB bearbeiten</a>]</font></font></p>
        <p> <font face=\"Verdana, Arial, Helvetica, sans-serif\">
          <select name=\"Sender\">
            <option value=\"\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;bitte w&auml;hlen&gt;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</option>"
# hier absender-tb einf�gen
idx=1
while read line
do
    if [ ! -z "$line" ]
    then
        num=`echo "$line"|cut -d= -f1`
        # pers=`echo "$line"|cut -d= -f2|tr -d '\n'`
        pers=`echo "$line"|cut -d= -f2`
        if [ "$idx" = "$YAPSGUI_SENDER_STD" ]; then
            echo "          <option value=\"$num\" selected>$pers</option>"
        else
            echo "          <option value=\"$num\">$pers</option>"
        fi
        idx=`/usr/bin/expr $idx + 1`
    fi
done < $YAPSGUI_SENDER_TB
echo "        </select><br><input type=\"text\" name=\"SenderM\" size=\"20\">
          </font></p>
      </td>
      <td width=\"1%\" align=\"center\"> 
        <p><font face=\"Verdana, Arial, Helvetica, sans-serif\"><b>Empf&auml;nger<br>
          </b><font size=\"1\">[<a href=yapsGUI_edit.cgi?action=edit&tb=recipient>TB bearbeiten</a>]</font></font></p>
        <p> <font face=\"Verdana, Arial, Helvetica, sans-serif\"> 
          <select name=\"Recipient\">"
echo "            <option value=\"\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;bitte w&auml;hlen&gt;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</option>"
#hier empf�nger-tb einf�gen
idx=1
while read line
do
    if [ ! -z "$line" ]
    then
        num=`echo "$line"|cut -d= -f1`
        # pers=`echo "$line"|cut -d= -f2|tr -d '\n'`
        pers=`echo "$line"|cut -d= -f2`
        if [ "$idx" = "$YAPSGUI_RECIPIENT_STD" ]; then
            echo "          <option value=\"$num\" selected>$pers</option>"
        else
            echo "          <option value=\"$num\">$pers</option>"
        fi
        idx=`/usr/bin/expr $idx + 1`
    fi
done < $YAPSGUI_RECIPIENT_TB
echo "        </select>
          <br>
          <input type=\"text\" name=\"RecipientM\" size=\"20\">
          </font></p>
      </td>
      <td width=\"98%\" align=\"center\"> 
        <p><font face=\"Verdana, Arial, Helvetica, sans-serif\"><b>Nachricht</b></font></p>
        <p> <font face=\"Verdana, Arial, Helvetica, sans-serif\"> 
          <textarea name=\"Message\" cols=\"40\" rows=\"4\" onChange=\"return CheckKey()\"  onKeyDown=\"return CheckKey()\" onKeyUp=\"return CheckKey()\" onFocus=\"return CheckKey()\"></textarea>
          </font></p>
      </td>
    </tr>
    <tr valign=\"top\"> 
      <td align=\"center\"><font face=\"Verdana, Arial, Helvetica, sans-serif\"><nobr>Debuglevel 
        <select name=\"Debug\">"
# debuglevel
if [ "$YAPSGUI_DEBUG" = "0" ]
 then
  Debug_0_Selected="selected"
fi

if [ "$YAPSGUI_DEBUG" = "1" ]
 then
  Debug_1_Selected="selected"
fi

if [ "$YAPSGUI_DEBUG" = "2" ]
 then
  Debug_2_Selected="selected"
fi

if [ "$YAPSGUI_DEBUG" = "3" ]
 then
  Debug_3_Selected="selected"
fi

if [ "$YAPSGUI_DEBUG" = "4" ]
 then
  Debug_4_Selected="selected"
fi

  echo "          <option value=\"0\" $Debug_0_Selected>0</option>
          <option value=\"1\" $Debug_1_Selected>1</option>
          <option value=\"2\" $Debug_2_Selected>2</option>
          <option value=\"3\" $Debug_3_Selected>3</option>
          <option value=\"4\" $Debug_4_Selected>4</option>"
echo "        </select></nobr></font></td>
      <td align=\"center\"><font face=\"Verdana, Arial, Helvetica, sans-serif\"><nobr>
        <input type=\"checkbox\" name=\"RequestConfirm\" value=\"1\">
        Empfangsbest&auml;tigung</nobr></font></td>
      <td align=\"center\"> 
        <p><font face=\"Verdana, Arial, Helvetica, sans-serif\">Noch 
          <input type=\"text\" name=\"Remaining\" disabled size=\"3\" value=\"160\">
          Zeichen</font></p>
      </td>
    </tr>
  </table>"
  
echo "  <p>&nbsp;</p>
  <center>
    <font face=\"Verdana, Arial, Helvetica, sans-serif\">
    <input type=\"reset\" name=\"Reset\" value=\"Reset\">
    <input type=\"submit\" value=\"Nachricht verschicken\">
    </font>
  </center>
</form>
</body></html>"

