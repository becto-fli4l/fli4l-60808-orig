#!/bin/sh
#####################################################################
#   File:         /srv/www/admin/phonebook.cgi                      #
#   Creationn:    20.06.2007 jk                                     #
#   Last Update:  $Id$                                              #
#####################################################################
. /srv/www/include/cgi-helper
show_html_header "Phonebook"
. /var/run/phonebook.conf

if [ ! -f /var/run/phonebook.cgi.option-$REMOTE_USER ]
then
 cp /var/run/phonebook.cgi.option /var/run/phonebook.cgi.option-$REMOTE_USER 2> /dev/null
fi

 if [ "$FORM_action" != "" ]
 then
  echo $( sed "s/&action=[^&]*//g;s/\$/\&action=$FORM_action/" /var/run/phonebook.cgi.option-$REMOTE_USER ) > /var/run/phonebook.cgi.option-$REMOTE_USER
 else
  FORM_action=$( sed -n "s/.*&action=\([^&]*\).*/\1/p" /var/run/phonebook.cgi.option-$REMOTE_USER )
  if [ "$FORM_action" = "" ]
  then
   FORM_action="opt_nr2name"
  fi
 fi

_opt_nr2name="Nummer zu Name"
_opt_phonebook="Phonebook"
_opt_statistik="Statistik"
_opt_log="Log"
show_tab_header "$_opt_nr2name" "$myname?action=opt_nr2name" \
                "$_opt_phonebook" "$myname?action=opt_phonebook" \
                "$_opt_statistik" "$myname?action=opt_statistik" \
                "$_opt_log" "$myname?action=opt_log"
if [ "$FORM_action" != "opt_nr2name" ]
 then echo "<form action='$0' method='get' align='right'><input type='submit' value='Ansicht Aktualisieren'></form>"
fi
if [ "$FORM_action" = "opt_log" ]
 then echo "<form action='$0?action=opt_log' method='get'><input name='log' value='del' type='hidden'><input type='submit' value='Log L&ouml;schen'></form>"
fi

if [ "$FORM_action" = "opt_nr2name" ]
then
Controll="ok"
 if [ "$FORM_Nummer" != "" ]
 then
  wert=$(echo "$FORM_Nummer"  | sed -e 's/[^1234567890+]//g;s/^+49/0/;s/^+/00/;s/+//g' )
  echo "Schlage <b>$wert</b> nach...<br>"
  if [ "$FORM_suchen_phonebook" = "Nein" -a "$FORM_suchen_internet" = "Nein" ]
  then
  show_error "" "...wenn ich nirgendwo suchen soll, kann ich auch nichts finden! ;-)"
  fi
   if [ "$FORM_suchen_phonebook" != "Nein" ]
   then
    echo "Schlage im Phonebook nach...<br><b>"
    grep "^$wert=" /etc/phonebook
    if [ "$?" = "1" ]
    then
    show_error "" "</b>...im Phonebook konnte nichts gefunden werden!<br>"
    else
     echo "</b><br>"
    fi
    phonebook_checkbox_J="checked"
    phonebook_checkbox_N=""
   else
    phonebook_checkbox_J=""
    phonebook_checkbox_N="checked"
   fi
   if [ "$FORM_suchen_internet" != "Nein" ]
   then
    echo "Schlage im Internet nach...<br>"
    ergebnis=$( phonebookInetSearch.sh "$wert" -azh  2> /tmp/phonebook.error-$$ )
    fehler=$( sed "s/ä/\&auml;/g;s/Ä/\&Auml;/g;s/ö/\&ouml;/g;s/Ö/\&ouml;/g;s/ü/\&uuml;/g;s/Ü/\&Uuml;/g;s/ß/\&szlig;/g;" /tmp/phonebook.error-$$ )
    rm /tmp/phonebook.error-$$
    if [ "$fehler" != "" ]
    then
     echo "<i>$fehler</i><br>"
     show_error "" "...im Internet konnte nichts gefunden werden!<br>"
    else
     echo "$ergebnis<hr>"
     button="Ins Phonebook eintragen"
     grep -q "^$wert=" /etc/phonebook
     if [ "$?" = "0" ]
     then
      echo "<b>Achtung:</b>Im Phonebook befinden sich bereits Eintr&auml;ge mit dieser Nummer. Es wird empfohlen,"
      echo "diese zu l&ouml;chen bevor sie die gefundenen Daten &uuml;bernehmen. Es handelt sich um folgende Eintr&auml;ge:<br><pre>"
      grep "^$wert=" /etc/phonebook | sed "s/^/     /"
      echo "</pre>"
      echo "<form action='$0' method='get'>"
      echo "<input name='grep' value='^$wert=' type='hidden'>"
      echo "<input name='action' value='opt_phonebook' type='hidden'>"
      echo "<input type='submit' value='Betroffene anzeigen/editiern'>"
      echo "</form><br>"
      button="Trotzdem ins Phonebook eintragen"
     fi
     echo "<form action='$0' method='get'>"
     echo "<input name='addnummer' value='$wert' type='hidden'>"
     echo "<input name='addname' value='$( echo "$ergebnis" | tr -d "\n\r" | sed "s/<[^>]*>//g" )' type='hidden'>"
     echo "<input name='action' value='opt_phonebook' type='hidden'>"
     echo "<input type='submit' value='$button'></form>"
     #echo "<form action='$0' method='get'>"
     #echo "<input name='add' value='$wert' type='hidden'>"
     #echo "<input name='action' value='opt_nr2name' type='hidden'>"
     #echo "<input type='submit' value='$button'>"
     #echo "</form><br>"
    fi
    internet_checkbox_J="checked"
    internet_checkbox_N=""
   else
    internet_checkbox_J=""
    internet_checkbox_N="checked"
   fi
  echo "<br><br>"
 fi

 #Fr sinnvolle Werte bei den Ja/Nein Checkboxen zur Suche in Internet und Phonebook sorgen
 if [ "$internet_checkbox_J$internet_checkbox_N" != "checked" ]; then internet_checkbox_J="checked"; internet_checkbox_N=""; fi
 if [ "$phonebook_checkbox_J$phonebook_checkbox_N" != "checked" ]; then phonebook_checkbox_J="checked"; phonebook_checkbox_N=""; fi

 #Die Eingabemaske zum suchen
 echo "<form action='$0' method='get'>"
 echo "Name zu folgender Rufnummer suchen:"
 echo "<input name='Nummer' value='$wert' type='text'>"
 echo "<input name='action' value='opt_nr2name' type='hidden'><br>"
 echo "Suchen im Phonebook? <input name='suchen_phonebook' value='Ja' type='radio' $phonebook_checkbox_J>Ja &nbsp;"
 echo "<input name='suchen_phonebook' value='Nein' type='radio' $phonebook_checkbox_N>Nein<br>"
 echo "Suchen im Internet <i>(Das &Ouml;rtliche)</i>? <input name='suchen_internet' value='Ja' type='radio' $internet_checkbox_J>Ja &nbsp;"
 echo "<input name='suchen_internet' value='Nein' type='radio' $internet_checkbox_N>Nein<br>"
  #echo "<input name='ins_phonebook' value='yes' type='checkbox' checked disabled> gefundenen Namen ins Phonebook &uuml;bernehmen. <i>(disabled)</i><br>'
 echo "<center><input type='submit' value='suchen'></center>"
 echo "</form><br>"

fi


if [ "$FORM_action" = "opt_phonebook" ]
then
Controll="ok"
 
 FORM_zeile=$( echo "$FORM_zeile" | sed "s/[^0-9]//g" )  #Zur Sicherheit
 if [ "$FORM_change$FORM_changename" != "" -a "$FORM_zeile" != "" ]
 then
  change="$FORM_change=$FORM_changename"
  #?dert die Zeile $FORM_zeile zu $change
  show_info "" "Zeile $FORM_zeile wird ge&auml;ndert zu: $change<br>"
  echo "Ändere Zeile $FORM_zeile vorher: $( sed -n "$FORM_zeile p" /etc/phonebook ) Zu: $change" >> $PHONEBOOK_LOG
sed -e "$FORM_zeile a\
$change" /etc/phonebook > /tmp/phonebookneu-$$
  sed "$FORM_zeile d" /tmp/phonebookneu-$$ > /etc/phonebook
  rm /tmp/phonebookneu-$$
 fi

 FORM_view=$( echo "$FORM_view" | sed "s/[^0-9]//g" )  #Zur Sicherheit
 if [ "$FORM_view" != "" ]
 then
  zeile=$( sed -n "$FORM_view p" /etc/phonebook | sed "s/&/\&amp;/g;s/\"/\&quot;/g;s/</\&lt;/g;s/>/\&gt;/g" )
  #?derungs Eingabemaske
  zeilenummer=$( echo "$zeile" | sed "s/^\([^#=]*\)=.*$/\1/" )
  zeilename=$( echo "$zeile" | sed "s/^\([^#=]*=\)//" )
  echo "<table><tr><td><b>Bearbeiten:</b></td><td>"
  echo "<form name='Eingabe' action='$0' method='get'>"
  echo "<input name='change' value='$zeilenummer' type='text' size='12'>"
  echo "<input name='changename' value='$zeilename' type='text' size='40'>"
  echo "<input name='action' value='opt_phonebook' type='hidden'>"
  echo "<input name='zeile' value='$FORM_view' type='hidden'>"
  echo "<input type='submit' value='speichern'>"
  echo "</form>"
  echo "</td></tr><tr><td></td><td>"
  echo "<form name='NrZuName' action='$0' method='get' onsubmit='document.NrZuName.Nummer.value=document.Eingabe.change.value;'>"
  echo "<input name='Nummer' value='' type='hidden'>"
  echo "<input name='action' value='opt_nr2name' type='hidden'>"
  echo "<input name='suchen_phonebook' value='Nein' type='hidden'>"
  echo "<input name='suchen_internet' value='Ja' type='hidden'>"
  echo "<input type='submit' value='Nummer im Internet suchen'>"
  echo "</form>"
  echo "</td></tr></table>"

  $FORM_kill=""   #Da sich beim Killen die Zeilennummern ?dern wrden.
 fi

 FORM_kill=$( echo "$FORM_kill" | sed "s/[^0-9]//g" )  #Zur Sicherheit
 if [ "$FORM_kill" != "" ]
 then
  #Löchen einer Zeile - definiert über Zeilennummer
  zeile=$( sed -n "$FORM_kill p" /etc/phonebook )
  echo "L&ouml;sche Zeile: $zeile" >> $PHONEBOOK_LOG
  sed "$FORM_kill d" /etc/phonebook > /tmp/phonebookneu-$$
  cat /tmp/phonebookneu-$$ > /etc/phonebook
  rm /tmp/phonebookneu-$$
  zeile=$( echo "$zeile" | sed "s/&/\&amp;/g;s/\"/\&quot;/g;s/</\&lt;/g;s/>/\&gt;/g;s/'/&39;/g" )
show_info "" "<b>L&ouml;sche Zeile:</b> $zeile"
  echo "<form action='$0' method='get'>"
  echo '<input name="action" value="opt_phonebook" type="hidden">'
  echo "<input name='add' value='$zeile' type='hidden' size='40'>"
  echo '<input type="submit" value="wieder eintragen">'
  echo '</form><br>'

 fi

 if [ "$FORM_neuerEintrag" = "ja" ]
 then
  #Formular für neuen Eintrag:
  echo "<a name='neuereintrag'><b>Neuen Eintrag ins Phonebook einf&uuml;gen:</b></a>"
  echo "<form action='$0' method='get'><table><tr>"
  echo '<td>Nummer: <input name="addnummer" value="0123456789" type="text" size="20"></td>'
  echo '<td>Name:</td><td><input name="addname" value="Mister Mustermann" type="text" size="20"></td>'
  echo '<td><input name="action" value="opt_phonebook" type="hidden">'
  echo '<input type="submit" value="hinzuf&uuml;gen"></td></tr></table>'
  echo '</form><br><hr>'
 else
  echo "<form action='$0' method='get' align='right'><input name='action' value='opt_phonebook' type='hidden'>"
  echo "<input name='neuerEintrag' value='ja' type='hidden'><input type='submit' value='Neuen Eintrag hinzuf&uuml;gen'></form>"
 fi

 # Hinzufügen einer übergebenen Zeile am Ende des Phonebooks
 if [ "$FORM_addname$FORM_addnummer" != "" ] 
 then
  phonebookeintrag="$FORM_addnummer=$FORM_addname"
  show_info "" "<b>F&uuml;ge Zeile ein:</b> $phonebookeintrag"
  echo "Füge Zeile ein: $phonebookeintrag" >> $PHONEBOOK_LOG
  echo -e "$phonebookeintrag" >> /etc/phonebook
 fi

 #Phonebook anzeigen:

 #Die Sortierung eventuell aus der Datei /var/run/phonebook.cgi.option-$REMOTE_USER nachladen
 if [ "$FORM_sort" != "" ]
 then
  echo $( sed "s/&sort=[^&]*//g;s/\$/\&sort=$FORM_sort/" /var/run/phonebook.cgi.option-$REMOTE_USER ) > /var/run/phonebook.cgi.option-$REMOTE_USER
 else
  FORM_sort=$( sed -n "s/.*&sort=\([^&]*\).*/\1/p" /var/run/phonebook.cgi.option-$REMOTE_USER )
 fi


 # Die Ansicht-Checkboxen interpretieren eventuell aus /var/run/phonebook.cgi.option-$REMOTE_USER und anzeigen
 if [ "$FORM_ansicht" = "" ]
 then
  FORM_ansicht=$( sed -n "s/.*&ansicht=\([^&]*\).*/\1/p" /var/run/phonebook.cgi.option-$REMOTE_USER )
 else
  if [ "$FORM_ansicht" != "klassisch" ]
   then FORM_ansicht="komfort"
  fi
  echo $( sed "s/&ansicht=[^&]*//g;s/\$/\&ansicht=$FORM_ansicht/" /var/run/phonebook.cgi.option-$REMOTE_USER ) > /var/run/phonebook.cgi.option-$REMOTE_USER
 fi
 if [ "$FORM_ansicht" != "klassisch" ]
  then ansicht_checkbox_Ko="checked"; ansicht_checkbox_Kl=""
  else ansicht_checkbox_Ko=""; ansicht_checkbox_Kl="checked"
 fi

 # Die Suchanfrage eventuell aus der Datei /var/run/phonebook.cgi.option-$REMOTE_USER nachladen und erweitern, damit sie 
 # nicht durch die Nummerierung gestöt wird.
 # Keine Suche wird duch "&grep=.*" bzw. "&grep=.%2A" ausgedrckt, da grep="" nicht von ungesetztem grep=...
 # unterschieden werden kann.
 if [ "$FORM_grep" = "" ]
  then FORM_grep=$( sed -n "s/.*&grep=\([^&]*\).*/\1/p" /var/run/phonebook.cgi.option-$REMOTE_USER )
  else echo $( sed "s/&grep=[^&]*//g;s/\$/\&grep=$FORM_grep/" /var/run/phonebook.cgi.option-$REMOTE_USER ) > /var/run/phonebook.cgi.option-$REMOTE_USER
 fi
 grepstring=$( echo "$FORM_grep" | sed "s/^[^^]/.*&/;s/^^//;s/^/[ 	]*[0-9][0-9]* /" )

 if [ "$FORM_maps" = "" ]
  then FORM_maps=$( sed -n "s/.*&maps=\([^&]*\).*/\1/p" /var/run/phonebook.cgi.option-$REMOTE_USER )
  else echo $( sed "s/&maps=[^&]*//g;s/\$/\&maps=$FORM_maps/" /var/run/phonebook.cgi.option-$REMOTE_USER ) > /var/run/phonebook.cgi.option-$REMOTE_USER
 fi

 # Suchfeld anzeigen
 echo "<table><tr><td><form>Suchen:"
 echo "<input name='action' value='opt_phonebook' type='hidden'><input name='sort' value='$FOR_sort' type='hidden'>"
 echo "<input name='grep' value='$FORM_grep' type='text' size='20'>"
 echo "<input type='submit' value='Suchen'></form></td>"
 #Direktsuche Hanynummern
 echo "<td><form><input name='action' value='opt_phonebook' type='hidden'><input name='sort' value='$FOR_sort' type='hidden'>"
 echo "<input name='grep' value='^0*01[5-7][0-9][0-9]*=' type='hidden'>"
 echo "<input type='submit' value='Handynummern'></form></td>"
 #Alle anzeigen = Suche beenden
 echo "<td><form><input name='action' value='opt_phonebook' type='hidden'><input name='sort' value='$FOR_sort' type='hidden'>"
 echo "<input name='grep' value='.*' type='hidden'>"
 echo "<input type='submit' value='Alle anzeigen'></form>"
 echo "</td></tr></table>"
 #kenntlich machen, da?nicht alle Datens?ze angezeigt werden
 if [ "$FORM_grep" != ".*" ]
  then  echo "<b>Suchergebnis f&uuml;r: '<i>$FORM_grep</i>'</b><br>"
 fi

 echo "<table><tr><td>Zum Sortieren bitte auf '<i>Nummer</i>' oder '<i>Name</i>' im Tabellenkopf klicken.</td>"
 echo "<td><form action='$0' method='get'><input name='action' value='opt_phonebook' type='hidden'><input name='sort' value='Unsortiert' type='hidden'><input type='submit' value='Sortierung aufheben'></form></td></tr></table>"

 if [ -f "/etc/phonebook" ]
 then
  # Nach NamenSortiertes Phonebook:  sed "h;s/^[^=]*=//;s/\$/==/;y/abcdefghijklmnopqrstuvwxyzäöüß/ABCDEFGHIJKLMNOPQRSTUVWXYZÄÖÜS/;s/^/~/;G" /etc/phonebook  | tr -d "\n\r" | tr "~" "\n" | sort | sed "s/.*==//"
  # z-a Sortierung mit sort -r    Soriteren von Nummern mit sort-n
  # Ansicht greift nach der Sortierung

  if [ "$FORM_ansicht" != "klassisch" ]
   then ansichtErzeuger=";s/^[ 	]*//;s#\([0-9][0-9]*\)[ 	]\(.*\)#\2</td><td><form action=\"$0\" method=\"get\"><input name=\"action\" value=\"opt_phonebook\" type=\"hidden\"><input name=\"view\" value=\"\1\" type=\"hidden\"><input type=\"submit\" value=\"bearbeiten\"></form></td><td><form action=\"$0\" method=\"get\"><input name=\"action\" value=\"opt_phonebook\" type=\"hidden\"><input name=\"kill\" value=\"\1\" type=\"hidden\"><input type=\"submit\" value=\"l\&ouml;schen\"></form></tr>#;s!^\([^=#<]*\)\([#<]\)!\1=\2!;s!=!</td><td>!;s/^/<tr><td>/;s!\(#[^<]*\)<!<font color='gray'>\1</font><!"
        ansichtTrenner="</th><th>"
        pregrepstring="[ 	]*[0-9][0-9]* [^#=]*[^#0][^#=]*="
   else ansichtErzeuger=";s/^[ 	]*//;s#\([0-9][0-9]*\)[ 	]\(.*\)#<tr><td>\2</td><td><small><a href=\"$0?action=opt_phonebook\&view=\1\">bearbeiten</a></td><td><a href=\"$0?action=opt_phonebook\&kill=\1\">l\&ouml;schen</a></small></td></tr>#;s!\(#[^<]*\)<!<font color='gray'>\1</font><!"
        ansichtTrenner="="
        pregrepstring=""
  fi
  
  #Google-Maps Links erzeugen: http://maps.google.de/maps?f=q&hl=de&q=%s&btnG=Adressen+suchen
  if [ "$FORM_maps" = "on" ]
   then ansichtErzeuger="$ansichtErzeuger;s|\([a-zA-ZÄÖÜäöüß][a-zA-ZÄÖÜäöüß\._-]* [0-9][0-9]*[;,] [0-9][0-9][0-9][0-9][0-9] [a-zA-ZÄÖÜäöüß][a-zA-ZÄÖÜäöüß\._ -]*\)|<a href='http://maps.google.de/maps?f=q\&hl=de\&q=\1\&btnG=Adressen+suchen'>\1</a>|"
  fi

  #Die Erzugung der Phonebook-Ansicht erfolgt nach einem bestimmten Schema:
  # 0) Die HTML-Tabellenkopfzeile erzuegen
  # 1) Nummerierung (da später über die Zeilennummer das Bearbeiten und Löschen funktioniert
  # 2) Filterung von Kommentarzeilen mit pregrepstring und gemäß der Suchabfrage mit grepstring
  # 3) Voranstellen eines Sortierstrings (suboptimal, da die Zeichen nicht aufgefüllt werden)
  # 4) Sortieren mit sort
  # 5) Aufräumen und dann den ansichtErzeuger zur HTMl-Erzeugung verwenden
  echo "<table class=MsoTableGrid border=0 cellspacing=0 cellpadding=0>" 
  case "$FORM_sort" in
   "name")
     echo "<tr><th><a href=\"$0?action=opt_phonebook&sort=number\">Nummer</a>"
     echo "$ansichtTrenner"
     echo "<a href=\"$0?action=opt_phonebook&sort=nameR\"><font color='red'>Name &darr;</font></a></th>"
     echo "<th>bearbeiten</th><th>l&ouml;chen</th></tr>"
     nl -ba -s" " /etc/phonebook | grep "$pregrepstring" | grep "$grepstring" |
     sed "s/~/_/g;h;s/#.*//;s/^[^=]*//;s/=//;s/\$/==/;y/abcdefghijklmnopqrstuvwxyzäöüß/ABCDEFGHIJKLMNOPQRSTUVWXYZÄÖÜS/;G;s/\$/~/" | tr -d "\n" | tr "~" "\n"  | sort | 
     sed "s/.*==//$ansichtErzeuger"
   ;;
   "nameR")
     echo "<tr><th><a href=\"$0?action=opt_phonebook&sort=number\">Nummer</a>"
     echo "$ansichtTrenner"
     echo "<a href=\"$0?action=opt_phonebook&sort=name\"><font color='red'>Name &uarr;</font></a></th>"
     echo "<th>bearbeiten</th><th>l&ouml;chen</th></tr>"
     nl -ba -s" " /etc/phonebook | grep "$pregrepstring" | grep "$grepstring" |
     sed "s/~/_/g;h;s/#.*//;s/^[^=]*//;s/=//;s/\$/==/;y/abcdefghijklmnopqrstuvwxyzäöüß/ABCDEFGHIJKLMNOPQRSTUVWXYZÄÖÜS/;G;s/\$/~/" | tr -d "\n" | tr "~" "\n"  | sort -r | 
     sed "s/.*==//$ansichtErzeuger" 
   ;;
   "number")
     echo "<tr><th><a href=\"$0?action=opt_phonebook&sort=numberR\"><font color='red'>Nummer &darr;</font></a>"
     echo "$ansichtTrenner"
     echo "<a href=\"$0?action=opt_phonebook&sort=name\">Name</a></th>"
     echo "<th>bearbeiten</th><th>l&ouml;chen</th></tr>"
     nl -ba -s" " /etc/phonebook | grep "$pregrepstring" | grep "$grepstring" | 
     sed "s#^ *\([0-9][0-9]*\) *\([^ ].*\)\$#\2 \1#" | sort -n |
     sed "s#\(.*\) \([0-9][0-9]*\)#\2 \1#$ansichtErzeuger"
   ;;
   "numberR")
     echo "<tr><th><a href=\"$0?action=opt_phonebook&sort=number\"><font color='red'>Nummer &uarr;</font></a>"
     echo "$ansichtTrenner"
     echo "<a href=\"$0?action=opt_phonebook&sort=name\">Name</a></th>"
     echo "<th>bearbeiten</th><th>l&ouml;chen</th></tr>"
     nl -ba -s" " /etc/phonebook | grep "$pregrepstring" | grep "$grepstring" |
     sed "s#^ *\([0-9][0-9]*\) *\([^ ].*\)\$#\2 \1#" | sort -nr |
     sed "s#\(.*\) \([0-9][0-9]*\)#\2 \1#$ansichtErzeuger"
   ;;
   *)
     echo "<tr><th><a href=\"$0?action=opt_phonebook&sort=number\">Nummer</a>"
     echo "$ansichtTrenner"
     echo "<a href=\"$0?action=opt_phonebook&sort=name\">Name</a></th>"
     echo "<th>bearbeiten</th><th>l&ouml;chen</th></tr>"
     nl -ba -s" " /etc/phonebook | grep "$pregrepstring" | grep "$grepstring" | sed "$ansichtErzeuger"
   ;;
  esac
  echo "</table>"
 else
  show_error "" "Es konnte kein Phonebook auf dem Server gefunden werden!<form action=\"$0\" method=\"get\"><input name=\"action\" value=\"createphonebook\" type=\"hidden\"><input type=\"submit\" value=\"Phonebook erstellen\"></form>"
 fi

 # Auswahl Klassisch oder Komfort
 echo "<hr>"
 echo "<form><table><tr><td><b>Ansicht:</b></td>"
 echo "<input name='action' value='opt_phonebook' type='hidden'>"
 echo "<td><input name='ansicht' value='komfort' type='radio' $ansicht_checkbox_Ko>Komfort</input></td>"
 echo "<td><input name='ansicht' value='klassisch' type='radio' $ansicht_checkbox_Kl>Klassisch</input></td>"
 echo "<td><input type='submit' value='anwenden'></td>"
 echo "</tr></table></form>"
 if [ "$FORM_maps" = "on" ]
  then maps_on="checked"; maps_off=""
  else maps_on=""; maps_off="checked"
 fi
 echo "<form><table><tr><td><b>Adressen auf Google-maps linken:</b></td>"
 echo "<input name='action' value='opt_phonebook' type='hidden'>"
 echo "<td><input name='maps' value='on' type='radio' $maps_on>An</input></td>"
 echo "<td><input name='maps' value='off' type='radio' $maps_off>Aus</input></td>"
 echo "<td><input type='submit' value='anwenden'></td>"
 echo "</tr></table></form>"
fi



if [ "$FORM_action" = "opt_statistik" ]
 then
 Controll="ok"
 grep "^[0-9][0-9]*=" /etc/phonebook > /tmp/phonebook.1-$$
 AnzahlGesamt=$( grep -c "" /tmp/phonebook.1-$$ )
 AnzahlIntern=$( grep -c "^[0-9][0-9][0-9]=" /tmp/phonebook.1-$$ )
 grep -v "^[0-9][0-9][0-9]=" /tmp/phonebook.1-$$ > /tmp/phonebook.2-$$
 AnzahlSond1=$( grep -c "^0*0[8-9]00" /tmp/phonebook.2-$$ )
 AnzahlSond2=$( grep -c "^0*013" /tmp/phonebook.2-$$ )
 AnzahlSond3=$( grep -c "^0*018" /tmp/phonebook.2-$$ )
 grep -v "^0*0[8-9]00" /tmp/phonebook.2-$$ | grep -v "^0*018" | grep -v "^0*013" > /tmp/phonebook.1-$$
 AnzahlSond=$( expr "$AnzahlSond1" "+" "$AnzahlSond2" "+" "$AnzahlSond3" )
 AnzahlHandy=$( grep -c "^0*01[5-7][0-9][0-9]*=" /tmp/phonebook.1-$$ )
 grep -v "^0*01[5-7][0-9][0-9]*=" /tmp/phonebook.1-$$ > /tmp/phonebook.2-$$
 AnzahlRest=$( grep -c "" /tmp/phonebook.2-$$ )
 rm /tmp/phonebook.1-$$ /tmp/phonebook.2-$$
 #show_info "" "<table><tr><td>Gesamt:</td><td>$AnzahlGesamt</td></tr><tr><td>Handynummer:</td><td>$AnzahlHandy</td></tr><tr><td>Sonderrufnummern:</td><td>$AnzahlSond</td></tr><tr><td>Interne Nummern:</td><td>$AnzahlIntern</td></tr><tr><td>Festnetz und Rest:</td><td>$AnzahlRest</td></tr></table>"
 echo "<table><tr><td>Gesamt:</td><td>$AnzahlGesamt</td></tr>"
 echo "<tr><td>Handynummer:</td><td>$AnzahlHandy</td><td><a href='$0?action=opt_phonebook&grep=^0*01[5-7][0-9][0-9]*='>anzeigen</a></td></tr>"
 echo "<tr><td>Sonderrufnummern:</td><td>$AnzahlSond</td></tr>"
 echo "<tr><td>Interne Nummern:</td><td>$AnzahlIntern</td><td><a href='$0?action=opt_phonebook&grep=^[0-9][0-9][0-9]='>anzeigen</a></td></tr>"
 echo "<tr><td>Festnetz und Rest:</td><td>$AnzahlRest</td></tr>"
 echo "</table>"
fi
if [ "$FORM_action" = "opt_log" ]
then
 Controll="ok"
 if [ "$FORM_log" = "del" ]
 then
  echo "" >$PHONEBOOK_LOG
show_info "" "Log gel&ouml;scht"
 fi
 cat $PHONEBOOK_LOG | sed 's/$/<br>/' | sed "s!Lösche.*\$!<font color=\"red\">&!" | sed "s!Füge.*\$!<font color=\"green\">&!" | sed "s!Ändere.*\$!<font color=\"blue\">&!" | sed "s!:.*\$!</font>&!"
fi

if [ "$FORM_action" = "createphonebook" ]
then
Controll="ok"
show_info "" "Phonebook wurde erstellt"
echo "0=no number" >/etc/phonebook
echo "00=no number" >>/etc/phonebook
fi

# Reparieren von Phonebooks mit alten Sonderzeichen
if [ "$FORM_repair" = "semikolon" ]
then sed "s/;/ /g" /etc/phonebook > /tmp/phonebook-$$
     cat /tmp/phonebook-$$ > /etc/phonebook
     rm /tmp/phonebook-$$
     Controll="ok"
     show_info "" "Wandle Semikola in Leerzeichen."
fi
if [ "$FORM_repair" = "vertical" ]
then sed "s/|/ /g" /etc/phonebook > /tmp/phonebook-$$
     cat /tmp/phonebook-$$ > /etc/phonebook
     rm /tmp/phonebook-$$
     Controll="ok"
     show_info "" "Wandle '|' in Leerzeichen."
fi
if [ "$FORM_repair" = "alt255" ]
then sed "s// /g" /etc/phonebook > /tmp/phonebook-$$         #komisches Sonderzeichen Alt-255 ""
     cat /tmp/phonebook-$$ > /etc/phonebook
     rm /tmp/phonebook-$$
     Controll="ok"
     show_info "" "Wandle Alt+255 in Leerzeichen."
fi
if [ "$FORM_repair" = "komischezeile" ]
then cat /etc/phonebook | tr -d"" > /tmp/phonebook-$$
     cat /tmp/phonebook-$$ > /etc/phonebook
     rm /tmp/phonebook-$$
     Controll="ok"
     show_info "" "Entferne komische Zeile, die manchmal auftritt."
fi
if [ "$FORM_repair" = "html" ]
then sed "s/<[^>]*>//g" /etc/phonebook > /tmp/phonebook-$$
     cat /tmp/phonebook-$$ > /etc/phonebook
     rm /tmp/phonebook-$$
     Controll="ok"
     show_info "" "Entferne HTML-Code aus /etc/phonebook."
fi
if [ "$FORM_repair" = "leerzeichen" ]
then sed "s/  */ /g" /etc/phonebook > /tmp/phonebook-$$
     cat /tmp/phonebook-$$ > /etc/phonebook
     rm /tmp/phonebook-$$
     Controll="ok"
     show_info "" "Fasse mehrfache Leerzeichen zusammen."
fi
if [ "$FORM_repair" = "kommata_semikola" ]
then sed "/^[^#]/ s/,/;/g" /etc/phonebook > /tmp/phonebook-$$
     cat /tmp/phonebook-$$ > /etc/phonebook
     rm /tmp/phonebook-$$
     Controll="ok"
     show_info "" "Alle Kommata werden in Semikola umgewandelt (außer in Kommentarzeilen)."
fi
if [ "$FORM_repair" = "kill_CR" ]
then cat /etc/phonebook | tr -d "\r" > /tmp/phonebook-$$
     cat /tmp/phonebook-$$ > /etc/phonebook
     rm /tmp/phonebook-$$
     Controll="ok"
     show_info "" "Alle &quot;CR&quot; = &quot;\r&quot; Returns werden entfernt."
fi
if [ "$FORM_repair" = "konvert_LF_CRLF" ]
then sed "s/$/$( echo -e "\r\n" )/" /etc/phonebook > /tmp/phonebook-$$
     cat /tmp/phonebook-$$ > /etc/phonebook
     rm /tmp/phonebook-$$
     Controll="ok"
     show_info "" "Alle &quot;LF&quot; = &quot;\n&quot; Returns werden zu (Windows kompatiblen) &quot;CR LF&quot; = &quot;\r\n&quot; konvertiert."
fi



if [ "$Controll" = "" ]
then
show_error "" "Link Existiert nicht!<br><i>Eventuell waren nicht alle notwendigen Felder ausgef&uuml;llt</i>"
fi
show_tab_footer
show_html_footer
