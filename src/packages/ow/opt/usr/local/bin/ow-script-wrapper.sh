#!/bin/sh
#------------------------------------------------------------------------------
# /usr/local/bin/ow-script-wrapper.sh                              __FLI4LVER__
#
# Creation:     04.02.2009 kmw <news4kmw@web.de>
# Modification: 22.01.2012 Roland Franke
# Last Update:  $Id$
#------------------------------------------------------------------------------

# Load configuration
. /var/run/ow.conf
: ${OW_USER_SCRIPT_STOP:='/var/lock/ow-userscript.stop'}
[ -f $OW_USER_SCRIPT_STOP ] && rm $OW_USER_SCRIPT_STOP
: ${OW_USER_SCRIPT_INTERVAL:='1'}

while [ -x "$OW_USER_SCRIPT" -a ! -e $OW_USER_SCRIPT_STOP ]
do
  /bin/sh $OW_USER_SCRIPT "$OW_OWFS_PATH"
  sleep $OW_USER_SCRIPT_INTERVAL
done
