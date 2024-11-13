#!/bin/sh
##-----------------------------------------------------------------------------
## cpmvrmlog/check.sh - check free blocks of /dev/ram
##
## Creation:     23.05.2004  lanspezi
## Last Update:  $Id$
##
##-----------------------------------------------------------------------------

# check /dev/ram
lecho=/usr/local/bin/cpmvrmlog/echo.sh
blksfree=`cat /var/run/cpmvrmlog/check.blks`
verbose=`cat /var/run/cpmvrmlog/verbose`

set -- `df | grep /$`

blksa=$4

if [ "$verbose" = "yes" ]
then
  $lecho "cpmvrmlog_check" "check free blocks on /dev/ram [$blksa free / $blksfree min]"
fi

if [ "$blksa" -lt "$blksfree" ]
then
  $lecho "cpmvrmlog_check" "not enough free blocks on /dev/ram [$blksa free / $blksfree min]"
  cpmvrmlog.sh
fi

set +x
