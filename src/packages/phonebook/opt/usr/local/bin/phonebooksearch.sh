#! /bin/sh
#-----------------------------------------------------------------------------
# /usr/local/bin/phonebookrefresh.sh - opt_phonebook              __FLI4LVER__
#
#
# Creation:     20.06.2007 jk
# Last Update:  $Id$
#-----------------------------------------------------------------------------

# Laden der config
. /var/run/phonebook.conf

if [ "$PHONEBOOK_DO_DEBUG" = "yes" ] ; then
   echo "`date "+%d.%m.%Y %H:%M:%S "` phonebooksearch.sh incomming Call from $1" >> $PHONEBOOK_LOG
fi
# setzen der Telefoummer als variable
telefonnummer=$1

# überprüfen, ob die Nummer im Phonebook auffindbar ist
name0="`grep ^$telefonnummer /etc/phonebook | sed -n 1p | cut -d= -f2 | sed -e 's/ /%20/g' `"
# Entfernen der 0 von einer Telefonanlage
if [ "$PHONEBOOKSEARCH_ADVANCE" = "yes" ] ; then
   #Sicherheitsï¿½berprï¿½fung das wirklich eine zusï¿½tzliche 0 durch telefonanlage bassiert ist
   if [ "`echo "$1" | cut -b-2`" = "00" ] ; then
     telefonnummer=`echo "$1" | cut -b2-`
     if [ "$PHONEBOOK_DO_DEBUG" = "yes" ] ; then
       echo "`date +%d.%m.%Y-%H:%M:%S:` phonebooksearch.sh Advance option set YES: $1 convert to $telefonnummer" >> $PHONEBOOK_LOG
     fi
   fi
fi

# Herausfiltern von Handynummern und 0190
if [ "`echo "$telefonnummer" | cut -b -3`" = "017" ] ; then exit ; fi
if [ "`echo "$telefonnummer" | cut -b -3`" = "016" ] ; then exit ; fi
if [ "`echo "$telefonnummer" | cut -b -3`" = "015" ] ; then exit ; fi
if [ "`echo "$telefonnummer" | cut -b -3`" = "019" ] ; then exit ; fi

 # Wenn Nummer nicht im Phonebook gefunden wurde...
if [ -z "$name0" ]
then
 #... Dialmode setzen fï¿½r Suche Im Internet
 if [ "$PHONEBOOKSEARCH_DIALAUTO" = "yes" ]
 then
  dialmode=""
  case $( { echo "dialmode"; echo "quit"; } | netcat localhost 5000 | cut -b4-6 ) in
   "off") dialmode="off" ;;
   "aut") dialmode="auto" ;;
   "man") dialmode="manual" ;;
   *) echo "$( date "+%d.%m.%Y-%H:%M:%S:" ) phonebooksearch.sh unbekannter Dialmode!" >&2; exit 2 ;;
  esac
  if [ "$( { echo "dialmode"; echo "quit"; } | netcat localhost 5000 | cut -b4-6 )" != "aut" ]
  then
   echo "$( date "+%d.%m.%Y-%H:%M:%S:" ) Konnte Dialmode nicht korrekt setzen!" >&2
   echo "Konnte Dialmode nicht korrekt setzen!"  >&2
   exit 2
  fi
 fi
 
 #if [ "$PHONEBOOK_DO_DEBUG" = "yes" ]
  #then
  echo "`date +%d.%m.%Y %H:%M:%S ` phonebooksearch.sh sucht in Internet Name zu '$telefonnummer'" >> $PHONEBOOK_LOG
 #fi
 if [ "$PHONEBOOKSEARCH_ADDNUMBERSTYLE" = "-az" ] ; then style="-"; fi
 if [ "$PHONEBOOKSEARCH_ADDNUMBERSTYLE" = "-azh" ] ; then style="-"; fi

 #Hier wird die eigentliche Suche durchgefï¿½hrt
 if [ "$style" != "-" ]
  then name1=$( phonebookInetSearch.sh "$telefonnummer" 2> /tmp/phonebook.error-$$ )
  else name1=$( phonebookInetSearch.sh "$telefonnummer" "$PHONEBOOKSEARCH_ADDNUMBERSTYLE" 2> /tmp/phonebook.error-$$ )
 fi
 fehler="$?"
 # Fehler 1 = Keine Teilnehmer oder nicht eindeutig
 # Fehler 2 = Verbindungsfehler
 if [ "$fehler" != "0" ]
  then
   name1=""
   echo $( cat /tmp/phonebook.error-$$ ) >&2
   echo "`date "+%d.%m.%Y %H:%M:%S "` phonebooksearch.sh empfängt Fehler von phonebookInetSearch.sh: $( cat /tmp/phonebook.error-$$ )" >> $PHONEBOOK_LOG
 fi
 rm /tmp/phonebook.error-$$
 
 # Dialmode zurücksetzen
 if [ "$PHONEBOOKSEARCH_DIALAUTO" = "yes" ]
 then
  { echo "dialmode $dialmode"; echo "quit"; } | netcat localhost 5000
  echo "$( date "+%d.%m.%Y %H:%M:%S " ) phonebooksearch.sh Change dialmode to $dialmode" >> $PHONEBOOK_LOG
 fi

 # Die 0 der Telefonanlage wieder mit in das Telefonbuch schreiben, damit die Namenserkennung auch dann weiter läuft
 if [ "$PHONEBOOKSEARCH_ADVANCE" = "yes" ]
  then telefonnummer=$1
 fi
 
 #nurnummer soll wirklich nur die Ziffern der Telephonnummer enthalten, wie sie auch von der ISDN Software später gemeldet werden.
 nurnummer=$( echo "$telefonnummer" | sed -e 's/[^1234567890+]//g;s/^+49/0/;s/^+/00/;s/+//g' )
 #Kommata in Semikola verwandeln, falls erwünscht
 if [ "$PHONEBOOKSEARCH_SEMIKOLON" = "yes" ]
  then name1=$( echo $name1 | sed "s/,/;/g" )
 fi
 # überprüfen, ob die Suche was gebracht hat, wenn ja, dann auch ins Telefonbuch eintragen
 if [ "$name1" !=  "" ]
 then
  #Nummer eintragen
  if [ "$PHONEBOOKSEARCH_ADDNUMBERSTYLE" != ""  ]
  then
   if [ "$style" != "-" ]
    then echoname=$(echo "$PHONEBOOKSEARCH_ADDNUMBERSTYLE" | sed -e 's/NAME/'"$name1"'/g' | sed -e 's/NUMMER/'$telefonnummer'/g')
    else echoname="$name1"
   fi
  else
   if [ "$PHONEBOOKSEARCH_ADDNUMBER" = "yes" ] #Bedeutet: außer dem Namen noch die Nummer eintragen
   then
    echoname="$name1 [$telefonnummer]"
   else
    echoname="$name1"
   fi
  fi
  echo "$nurnummer=$echoname" | sed 's/= /=/' >> /etc/phonebook
  echo "Füge Zeile ein: $nurnummer=$echoname" >> $PHONEBOOK_LOG
 else
 # Wenn PHONEBOOKSEARCH_AGAIN = no, Sperreintrag setzen, der verhindert, daß die Nummer erneut gesucht wird.
 # Wenn nur der HTTP-Error vorlag, dann doch kein Sperreintrag
  if [ "$fehler" != "2" -a "$PHONEBOOKSEARCH_AGAIN" != "yes" ]
  then
   echo "$nurnummer=$telefonnummer (Kein Teilnehmer gefunden)" | sed 's/= */=/' >> /etc/phonebook
  fi
 fi
fi
