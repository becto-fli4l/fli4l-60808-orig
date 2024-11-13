#!/bin/sh
#--------------------------------------------------------------------
# /usr/bin/check_bssid.sh - Run service in a loop
#
# Creation:    08.02.2003 tobig
# Last Update: $Id$
#--------------------------------------------------------------------
while true
do
  if [ `iwconfig $1 | grep "Cell" | cut -d " " -f 17` != $2 ] ; then 
  (
  date
  echo "BSSID "$2" von "$1" ist Falsch" 
  iwconfig $1 ap $2
  )
  fi
  sleep 2
done
