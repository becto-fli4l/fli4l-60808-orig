#!/bin/sh
##------------------------------------------------------------------------------
## c3Surf - login for services
## kill httpd
## parameter: none
## Creation:    2008 fs
## Last Update: $Id$
## mandatory: all files in /tmp/c3surf /var/run/c3surf
##-------------------------------------------------------------------------------
## Licence and conditions look at ~/config/c3surf.txt
##-------------------------------------------------------------------------------
if [ -f /var/run/c3surf_httpd.pid ]
then
    kill -HUP `cat /var/run/c3surf_httpd.pid`
fi
