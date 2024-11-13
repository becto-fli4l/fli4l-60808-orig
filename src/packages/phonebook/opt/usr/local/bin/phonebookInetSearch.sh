#!/bin/sh
#-----------------------------------------------------------------------------
# /usr/local/bin/phonebookInetSearch - opt_phonebook              __FLI4LVER__
#
# Creation:     20.06.2007 jk
# Last Update:  $Id$
#-----------------------------------------------------------------------------

nummer="$1"
telefonnummer=$(echo "$nummer"  | sed -e 's/[^1234567890+]//g;s/^+/00/;s/^0049/0/;s/+//g' )


if [ "`echo "$telefonnummer" | cut -b -3`" = "017" ] ; then echo "Nach Nummern mit Handyvorwahl wird nicht gesucht!" >&2 ; exit 1 ; fi
if [ "`echo "$telefonnummer" | cut -b -3`" = "016" ] ; then echo "Nach Nummern mit Handyvorwahl wird nicht gesucht!" >&2 ; exit 1 ; fi
if [ "`echo "$telefonnummer" | cut -b -3`" = "015" ] ; then echo "Nach Nummern mit Handyvorwahl wird nicht gesucht!" >&2 ; exit 1 ; fi
if [ "`echo "$telefonnummer" | cut -b -3`" = "019" ] ; then echo "Nach Nummern mit Handyvorwahl wird nicht gesucht!" >&2 ; exit 1 ; fi

tmpfile=/tmp/phonebook.antwort-$$

#wget -q -O "./2010_antwort_$telefonnummer.html" "http://www.dasoertliche.de/?form_name=search_inv&ph=$telefonnummer"
wget -q -O "$tmpfile" "http://www.dasoertliche.de/?form_name=search_inv&ph=$telefonnummer"

# Verbindungsfehler exit 2
if [ "$?" != "0" ]
then
  echo "Fehler bei der Suchanfrage (Netzwerk? Server?)!" >&2
  exit 2
fi

#keine oder zu viele Ergebnisse abfangen -> exit 1
nbData=$( grep -c "data =" "$tmpfile" )
if [ "$nbData" != "1" ]
then
  echo "Für diese Nummer wurde von 'das Oertliche' kein oder mehrere Teilnehmer gefunden!" >&2
  exit 1
fi

#eigentlich ist umcodierung nötig
#iconv --from-code=ISO-8859-15 --to-code=UTF8 "./2010_antwort_$telefonnummer.html" > "./2010_temp"

# Filter um nur den Namen auszugeben
filter="s/|.*//"

case "$2" in
 # Pipes durch Leerzeichen ersetzen (ms1958 30.4.2011)
 "-azh") filter="s/|/\&nbsp;/g"
       ;;
 "-az") filter="s/|/\&nbsp;/g"
       ;;
 "-az") filter="s/|/\&nbsp;/g"
       ;;
 *)
esac

sed -n "$tmpfile" -e "/var data = {/,/};/p;/class *= *\"nummer/,/\/div/p"  | sed "s/<[^>]*>//g;s/^[^>]*>//;s/<[^<]*\$//;s/&nbsp;//g" | 
tr -d "\t\n\r" | sed "s/\};/</g;s/%20/ /g" | tr "<" "\n" | 
sed "s/\(.*\)var data =.*pc: \"\([^\"]*\)\".*ci: \"\([^\"]*\)\".*na: \"\([^\"]*\)\".*st: \"\([^\"]*\)\".*/\4|\5, \2 \3|\1/" |
sed "s/[^0-9a-zA-Z]*\$//" | sed $filter

exit

#echo "   Name:"
#grep 'span class="preview_box"' "./2010_antwort_$telefonnummer.html" | sed "s/.*class=\"preview[^>]*>\([^<]*\)<.*/\1/"
#echo ""
#
#echo "   Adresse:"
#sed "./2010_antwort_$telefonnummer.html" -n -e "/class=\"strasse\"/,/<\/div>/p" | grep "</div>" | tr -d "\t" | sed "s/&nbsp;/ /g;s/<\/div.*//"
#
#echo ""
#echo "   Telephonnummer:"
#sed "./2010_antwort_$telefonnummer.html" -n -e "/class=\"nummer\"/,/<br\/>/p" | sed "s/[^0-9]//g" | grep "..*"