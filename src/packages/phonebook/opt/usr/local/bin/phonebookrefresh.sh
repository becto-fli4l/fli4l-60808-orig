#!/bin/sh
#-----------------------------------------------------------------------------
# /usr/local/bin/phonebookrefresh.sh - opt_phonebook              __FLI4LVER__
#
# Creation:     20.06.2007 jk
# Last Update:  $Id$
#-----------------------------------------------------------------------------

. /var/run/phonebook.conf
DATEI=/etc/phonebook

if [ -f $DATEI ]
then
 if [ -f /etc/phonebook.new-$$ ] ; then rm /etc/phonebook.new-$$; fi

 while read LINE; do
  #read LINE kann die nächste zeile lesen
  
  #überprüfe ob es keine Handy bzw Sonderrufnummer ist
  NumKurz=$( echo $LINE | cut -b -3)
  sonder="0"
  if [ "$NumKurz" = "015" ] ; then sonder="1"; fi
  if [ "$NumKurz" = "016" ] ; then sonder="1"; fi
  if [ "$NumKurz" = "017" ] ; then sonder="1"; fi
  if [ "$NumKurz" = "018" ] ; then sonder="1"; fi
  if [ "$NumKurz" = "019" ] ; then sonder="1"; fi
  # übrprüfe ob auch eine Nummer in der Zeile steht
  
  if [ "$( echo $LINE | grep '^0=')" != "" ] ; then sonder="1"; fi
  if [ "$( echo $LINE | grep '^00=')" != "" ] ; then sonder="1"; fi
  if [ "$( echo $LINE | grep '^[1-9]')" != "" ] ; then sonder="1"; fi
  if [ "$( echo $LINE | grep '^[0-9]')" = "" ] ; then sonder="2"; fi
  
  if [ "$sonder" != "0" ]
  then
   #übernehme Orginal Zeile
    if [ "$sonder" = "1" ]
    then
     echo "$LINE" >>/etc/phonebook.new-$$
    fi
 
    if [ "$sonder" = "2" ]
    then
     if [ "$1" != "-le" ]
     then
      echo "$LINE" >>/etc/phonebook.new-$$
     fi
    fi
  else
  
   #schneide alles nach der nummer ab
   Nummer=$( echo "$LINE" | cut -d= -f1 )
  
   #Suche $Nummer nun bei das Örtliche
   #ergeniss=$( /usr/local/bin/phonebookInetSearch.sh $Nummer )
 
   if [ "$PHONEBOOKSEARCH_ADDNUMBERSTYLE" = "-az" ] ; then style="-"; fi
   if [ "$PHONEBOOKSEARCH_ADDNUMBERSTYLE" = "-azh" ] ; then style="-"; fi
   if [ "$style" != "-" ]
   then
    name1=$( phonebookInetSearch.sh "$Nummer" 2> /tmp/phonebook.error-$$ )
   else
    name1=$( phonebookInetSearch.sh "$Nummer" "$PHONEBOOKSEARCH_ADDNUMBERSTYLE" 2> /tmp/phonebook.error-$$ )
   fi
 
   #Eintrï¿½e ins Phonebook bernehmen
   if [ "$?" = "0" ]
   then
    if [ "$PHONEBOOKSEARCH_SEMIKOLON" = "yes" ]
     then name1=$( echo $name1 | sed "s/,/;/g" )
    fi
    echo "$Nummer=$name1" >>/etc/phonebook.new-$$
   else
    echo "$LINE" >>/etc/phonebook.new-$$
    echo $( cat /tmp/phonebook.error-$$ ) >&2
   fi
  fi
 
 done < $DATEI
 
 if [ -f /etc/phonebook.new-$$ ]
 then
  rm /etc/phonebook
  cp /etc/phonebook.new-$$ /etc/phonebook
 fi
 
 if [ -f /etc/phonebook.new-$$ ] ; then rm /etc/phonebook.new-$$; fi
 
else
 echo "Die Datei '$DATEI' ist nicht vorhanden!"
fi
