#!/bin/sh
#------------------------------------------------------------------------------
# /srv/www/admin/ow-refresh.cgi - Device Tree Refresh              __FLI4LVER__
#
# Creation:     04.02.2009 kmw <news4kmw@web.de>
# Modification: 22.01.2012 Roland Franke
# Last Update:  $Id$
#------------------------------------------------------------------------------

. /srv/www/include/cgi-helper
. /var/run/ow.conf

StatusLED ()
  {
  local id=$1
  local status=$2
  #--------------
  # Invert signals with heading underscore in port name
  [ -n "`echo $id | grep '\-_'`" ] && status=`expr $status = 0`
  [ "$OW_INVERT_PORT_LEDS" = "yes" ] && \
    [ -n "`echo $id | grep '\-[_]*sensed[.0-9A-Z]*$'`" -o \
      -n "`echo $id | grep '\-[_]*PIO[.0-9A-Z]*$'`" -o \
      -n "`echo $id | grep '\-[_]*latch[.0-9A-Z]*$'`" ] && status=`expr $status = 0`
  local led=
  case $status in
    1|on|yes) led=green;;
    0|off|no) led=red;;
    warn)     led=yellow;;
    *)        led=grey;;
  esac
  echo 'parent.document.getElementById("led-'$id'").src="../img/ow-led-'$led'.gif";'
  }

Text ()
  {
  local id=$1
  shift
  local text=$*
  #--------------
  text=`echo "$text" | sed 's|^[ ]*||;s|[ ]*$||'`
  echo 'parent.document.getElementById("txt-'$id'").value="'"$text"'";'
  }

InputField ()
  {
  local id=$1
  local value=$2
  local unit=$3
  #--------------
  value=`echo "$value" | sed 's|^[ ]*||;s|[ ]*$||'`
  echo 'parent.document.getElementById("ifd-'$id'").value="'"$value"'";'
  }

Command ()
  {
  local id=$1
  local command=$2
  #--------------
  if [ -n "$command" ]
  then
    local level=`echo $command | cut -d- -f2-`
    # Invert signals with heading underscore in port name
    [ -n "`echo $id | grep '\-_'`" ] && level=`expr $level = 0`
    case $level in
      1) echo 'parent.document.getElementById("img-'$id'").src="../img/ow-toggle-up.gif";';;
      0) echo 'parent.document.getElementById("img-'$id'").src="../img/ow-toggle-down.gif";';;
      *) echo 'parent.document.getElementById("img-'$id'").src="../img/ow-toggle.gif";';;
    esac
    echo 'parent.document.getElementById("cmd-'$id'").value="'"`echo $id | cut -d- -f1`-$command"'";'
  fi
  }

ParseCommand ()
  {
  local command="$*"
  #--------------
  if [ -n "$command" ]
  then
    local file=
    local value=
    eval `echo "$command" | sed -n 's|^\([0-9A-Fa-f]\{2\}\)\([0-9A-Fa-f]\{12\}\)[0-9A-Fa-f]\{2\}-\([^-]*\)-\(.*\)$|file="'$OW_OWFS_PATH'/\1.\2/\3";value="\4"|p'`
    if [ -n "$file" -a -f $file ]; then echo -n "$value" > $file; fi
  fi
  }

if [ -x $OW_REFRESH_TEMP ]
then
  # start_time=`date +%s`
  local refresh=
  if [ $OW_REFRESH_INTERVAL -ge 5 ]; then refresh="refresh=$OW_REFRESH_INTERVAL;"; fi
  show_html_header "hidden-ow-refresh" "${refresh}url=$myname;showmenu=no;"
  echo action_call:$1 >> /tmp/ow-debug.log
  ParseCommand "$1"
  echo '<script language="JavaScript" type="text/javascript">'
  . $OW_REFRESH_TEMP
  # echo 'parent.document.getElementById("RefreshTime").value="'"$(expr $(date +%s) - $start_time)"'";'
  echo '</script>'
  show_html_footer
fi
