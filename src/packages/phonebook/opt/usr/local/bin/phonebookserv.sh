#! /bin/sh
#-----------------------------------------------------------------------------
# /usr/local/bin/phonebookrefresh.sh - opt_phonebook              __FLI4LVER__
#
# Creation:     20.06.2007 jk
# Last Update:  $Id$
#-----------------------------------------------------------------------------

# Laden der config
. /var/run/phonebook.conf

name=$( grep ^$1 /etc/phonebook | sed -n "1p" | sed "s#<[^<>]*>##g;s/^[^=]*=//" )
telefonnummer=$name
if [ "$name" = "" ] ; then
 telefonnummer=$1
fi
name=$( grep ^$2 /etc/phonebook | sed -n "1p" | sed "s#<[^<>]*>##g;s/^[^=]*=//" )
MSN=$name
if [ "$name" = "" ] ; then
 MSN=$2
fi

idx=1
while [ "$idx" -le "$PHONEBOOKSERV_N" ]
do
 eval wartemsn='$PHONEBOOKSERV_'$idx'_MSN'
 if [ "$wartemsn" = "$2" ]
 then
  eval Hostadress='$PHONEBOOKSERV_'$idx'_HOST'
  eval mode='$PHONEBOOKSERV_'$idx'_MODE'
  case "$mode" in
   "phonebookserv") 
     { sleep 1; echo $telefonnummer"="$MSN; } | netcat $Hostadress 5002
    ;;
    "sclient")
     echo "mAnruf von $telefonnummer an $MSN !" | netcat -u $Hostadress 1095 -w 1
    ;;
    "netsend")
     ip=""
     echo "$Hostadress" | grep "^[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}$"
     if [ "$?" = "0" ]
     then
      ip="$Hostadress"
     else
      ip=$( /bin/nmblookup -s /etc/smb.conf "$Hostadress" | grep -v "querying " | grep -v "name_query failed to find name" | cut -d" " -f1 )
      if [ "$ip" = "" ]
      then
       ip=$( nslookup "$Hostadress" | grep -v "127.0.0.1" | sed -n "s/$/,/;s/^.*[^0-9]\([0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}\)[^0-9].*$/\1/p" )
      fi
     fi
     if [ "$ip" = "" ]
     then
      echo "`date +%d.%m.%Y-%H:%M:%S:` phonebooksev.sh: Konnte Hostname '$Hostadress' nicht zu IP auflösen." >> $PHONEBOOK_LOG
      exit 1
     fi
     echo "Anruf von $telefonnummer an $MSN !" | /bin/smbclient  -s /etc/smb.conf -U "fli4l" -M "Anruf" -I "$ip" > /tmp/phonebookserv-$$ 2>&1
     #echo "Test!" | /bin/smbclient  -s /etc/smb.conf -U "fli4l" -M "Anruf" -I "192.168.0.16" > /tmp/phonebookserv-$$
     # mit 2>&1 kann man in Klammer eine Fehlermeldung finden:
     # bei inexistenter IP: (No route to host)
     # bei 192.168.178.1:   (Connection refused)
     if [ "$?" != "0" ]
     then
      fmeldung=$( grep "(" /tmp/phonebookserv-$$ | tr -d "\n" | sed "s/^.*(\(.*\)).*$/\1/" )
      echo "`date +%d.%m.%Y-%H:%M:%S:` phonebooksev.sh: Konnte Nachricht an '$Hostadress' nicht senden. (Fehler bei smbclient: $fmeldung)" >> $PHONEBOOK_LOG
      rm /tmp/phonebookserv-$$
      exit 1
     fi
     rm /tmp/phonebookserv-$$
     #samba-netsend $Hostadress "Anruf von $telefonnummer an $MSN !" > /tmp/phonebookserv.error-$$
     #grep -q "fehlgeschlagen" /tmp/phonebookserv.error-$$
     #if [ "$?" != "0" ]
     #then
     # echo "`date +%d.%m.%Y-%H:%M:%S:` phonebooksev.sh: netsend ERROR: $(cat /tmp/phonebookserv.error-$$)" >> $PHONEBOOK_LOG
     #fi
     #rm /tmp/phonebookserv.error-$$
    ;;
  esac
 fi

idx=`/usr/bin/expr $idx + 1`
done
