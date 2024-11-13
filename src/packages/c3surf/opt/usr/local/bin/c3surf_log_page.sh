#!/bin/sh
##------------------------------------------------------------------------------
## c3Surf - login for services
## html page logging
## parameter: $1         ; $2         ; $3      ; $4    ; $5        ; $6     ; $7
##            LogPath    ; IP-Adresse ; Browser ; fsid  ; fsvorname ; fsname ; fsmail
## param(1-3): mandatory
## param(4-7): optional
## Creation:    2007 fs
## Last Update: $Id$
## mandatory: all files in /tmp/c3surf /var/run/c3surf
##-------------------------------------------------------------------------------
## Licence and conditions look at ~/config/c3surf.txt
##-------------------------------------------------------------------------------
# all checks are off now, check C3SURF_DOLOG_PAGE = yes before you call this function
# read c3surf vars
# . /var/run/c3surf.conf
# if [ $C3SURF_DOLOG_PAGE = yes ]
# then
  echo "`date +%d.%m.%Y-%H:%M:%S`|$2|$3|$4 $5 $6 $7" >> "$1/c3surf_page.log"
# fi