#!/bin/sh
##-----------------------------------------------------------------------------
## cpmvrmlog.sh - copy, move or delete files
##
## Creation:     23.05.2004  lanspezi
## Last Update:  $Id$
##
##-----------------------------------------------------------------------------

for j in /var/run/cpmvrmlog/*.sh
do
    if [ -f $j ]
    then
        . $j
    fi
done
set +x
