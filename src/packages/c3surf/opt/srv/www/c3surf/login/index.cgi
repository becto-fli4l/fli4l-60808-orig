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

eval "`proccgi $*`"

# Package Vars
. /var/run/c3surf.conf

# disable page logging - it is too slow - look at 'no autologin'
# if [ "$C3SURF_DOLOG_PAGE" = "yes" ]
# then
#   /usr/local/bin/c3surf_log_page.sh "$C3SURF_LOG_PATH" "$REMOTE_ADDR" "$HTTP_USER_AGENT" "login/index.cgi" "$REMOTE_USER" "$FORM_action" "$FORM_time"
# fi

# Option: LOGINUSR
# Diese Seite erledigt die Nacharbeiten für das echte Login

# Tricky: wir wollen kein autologin, wenn der Client bereits Usr/Pwd Kombination kennt
case $FORM_action in
  login)
    case "x$FORM_time" in
    x)
      echo "Location: /index.cgi"
      echo
    ;;
    *)
      ta=`date +%s`
      td=`expr $ta - $FORM_time`
      # only send 401 if time difference between login request and entered password is less or equal 2 seconds
      case $td in
      0|1|2)
        echo 'Status: 401 Unauthorized'
        echo 'WWW-Authenticate: Basic realm="login"'
        echo
      ;;
      *)
        if [ -n "$REMOTE_USER" ]
        then
          # lese Daten der Anmeldung
          if [ -f $C3SURF_TMP_PATH/$REMOTE_ADDR.time ]
          then
            {
              read oldtime oldcount oldid oldrest
            } < $C3SURF_TMP_PATH/$REMOTE_ADDR.time
          else
            oldid=""
          fi
          # call the worker, logout is mandatory if there is someone else online - only one login per IP!
          if [ "$REMOTE_USER" != "$oldid" ]
          then
            /usr/local/bin/c3surf_worker.sh "doAutoLogout" "$REMOTE_ADDR"
          fi
          /usr/local/bin/c3surf_worker.sh "doLogin" "$REMOTE_ADDR" "$REMOTE_USER"
        fi
        echo "Location: /index.cgi"
        echo
      ;;
      esac
    ;;
    esac
  ;;
  *)
     echo "Location: /index.cgi"
     echo
  ;;
esac
