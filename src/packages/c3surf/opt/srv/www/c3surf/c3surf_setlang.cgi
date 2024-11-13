#!/bin/sh
##------------------------------------------------------------------------------
## c3Surf - login for services                                      __FLI4LVER__
##
## Creation:    07.01.2008 Frank Saurbier - c3Surf@arcor.de
## Last Update: $Id$
##
## Copyright (c) 2008-2010 - Frank Saurbier <c3surf@arcor.de>
## Copyright (c) 2010-2016 - Frank Saurbier, fli4l-Team <team@fli4l.de>
##
## Licence and conditions look at ~/config/c3surf.txt
##-------------------------------------------------------------------------------
# Package Vars
. /var/run/c3surf.conf

eval "`proccgi $*`"
# this script sets the default language for the login form

# page logging
if [ "$C3SURF_DOLOG_PAGE" = "yes" ]
then
  /usr/local/bin/c3surf_log_page.sh "$C3SURF_LOG_PATH" "$REMOTE_ADDR" "$HTTP_USER_AGENT" "setlang.cgi:" "$FORM_language"
fi

# only do work if a language code ist transmitted
# de, en, es
# /srv/www/c3surf/index.cgi
# /srv/www/c3surf/c3surf_status.cgi
if [ -n $FORM_language ]
then
  echo "c3surf_login_lang=\"$FORM_language\"" > /srv/www/c3surf/lang/default
fi
# go back to login page
echo "Location: /"
echo
exit 0
# we need Package Vars
# if we do it that way
#. /var/run/c3surf.conf
#echo "Location: http://$C3SURF_HTTPD_HOST_NAME/"
#echo
#exit 0

