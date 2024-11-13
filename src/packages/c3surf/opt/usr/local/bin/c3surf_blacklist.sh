#!/bin/sh
##------------------------------------------------------------------------------
## c3Surf - login for services
## add this MAC to blaclist
## parameter: $1 = mac-address, $2 action (add, remove) 3 optional: Host_IP
## Creation:    2007 fs
## Last Update: $Id$
## mandatory: all files in /tmp/c3surf /var/run/c3surf
##-------------------------------------------------------------------------------
## Licence and conditions look at ~/config/c3surf.txt
##-------------------------------------------------------------------------------
. /var/run/c3surf.conf
# check the mac
if [ -n "$1" -a "$1" != "-" ]
then
  # no capital letters
  tmp_arp_mac="$1"
  c3surf_mac="$(echo "$tmp_arp_mac" | sed -e 'y/ABCDEF/abcdef/')"
  # do it
  case $2 in
  add)
    if [ "$3" = "" ]
    then
      echo "$c3surf_mac" "none" "none" "none" >> $C3SURF_PERSISTENT_PATH/c3surf_mac.blacklist
    else  
      if [ -f "$C3SURF_TMP_PATH/$3.time" ]
      then
        # hole die Daten des Surfers
        {
          read c3surf_time c3surf_count c3surf_id c3surf_ip c3surf_mac c3surf_vorname c3surf_name c3surf_mail
        } < $C3SURF_TMP_PATH/$3.time
        echo "$c3surf_mac" "$c3surf_vorname" "$c3surf_name" "$c3surf_mail" >> $C3SURF_PERSISTENT_PATH/c3surf_mac.blacklist
      else
        echo "$c3surf_mac" "none" "none" "none" >> $C3SURF_PERSISTENT_PATH/c3surf_mac.blacklist
      fi
    fi  
  ;;
  remove)
    sed "/$c3surf_mac/d" $C3SURF_PERSISTENT_PATH/c3surf_mac.blacklist > /tmp/c3surf_mac.blacklist.$$
    mv -f /tmp/c3surf_mac.blacklist.$$ $C3SURF_PERSISTENT_PATH/c3surf_mac.blacklist
  ;;
  esac
  if [ "$C3SURF_WORKON_TMP" = "yes" ]
  then
    cp -f $C3SURF_PERSISTENT_PATH/c3surf_mac.blacklist $C3SURF_BLACKLIST_FILE
  fi
fi